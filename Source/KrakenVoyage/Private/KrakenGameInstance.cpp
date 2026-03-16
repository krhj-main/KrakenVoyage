// KrakenGameInstance.cpp

#include "KrakenGameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"

UKrakenGameInstance::UKrakenGameInstance()
{
}

void UKrakenGameInstance::Init()
{
	Super::Init();

	// Steam 연결 상태 확인 (로그만, 실패해도 게임은 동작)
	IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
	if (OSS)
	{
		UE_LOG(LogTemp, Log, TEXT("[GameInstance] Online Subsystem: %s"),
			   *OSS->GetSubsystemName().ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameInstance] No Online Subsystem! Using direct IP connection."));
	}
}

// ============================================================================
// 헬퍼: Session Interface 가져오기
// ============================================================================

IOnlineSessionPtr UKrakenGameInstance::GetSessionInterface() const
{
	// IOnlineSubsystem::Get() → Steam 플러그인이 활성화되어 있으면 Steam OSS 반환
	// 없으면 nullptr → IP 직접 연결로 폴백
	IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
	if (OSS)
	{
		return OSS->GetSessionInterface();
	}
	return nullptr;
}

// ============================================================================
// 방 만들기 (Host)
// ============================================================================

void UKrakenGameInstance::HostGame(const FString& PlayerName, int32 MaxPlayers)
{
	LocalPlayerName = PlayerName;
	PendingMaxPlayers = MaxPlayers;

	IOnlineSessionPtr Sessions = GetSessionInterface();

	// Steam 없으면 기존 IP 방식으로 폴백
	if (!Sessions.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("[GameInstance] No Steam. Using direct ServerTravel."));
		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(GameMapPath + TEXT("?listen"));
		}
		return;
	}

	// 기존 세션이 있으면 먼저 파괴 후 재생성
	// (이전 게임의 세션이 남아있을 수 있음)
	FNamedOnlineSession* ExistingSession = Sessions->GetNamedSession(NAME_GameSession);
	if (ExistingSession)
	{
		UE_LOG(LogTemp, Log, TEXT("[GameInstance] Destroying existing session first."));
		bPendingCreateAfterDestroy = true;

		DestroySessionHandle = Sessions->AddOnDestroySessionCompleteDelegate_Handle(
			FOnDestroySessionCompleteDelegate::CreateUObject(
				this, &UKrakenGameInstance::OnDestroySessionComplete));

		Sessions->DestroySession(NAME_GameSession);
		return;
	}

	// 세션 생성
	CreateSessionInternal(MaxPlayers);
}

// ============================================================================
// 내부: 실제 세션 생성
// ============================================================================

void UKrakenGameInstance::CreateSessionInternal(int32 MaxPlayers)
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions.IsValid()) return;

	// FOnlineSessionSettings: Steam에 "이런 방이 있다"고 알리는 설정
	FOnlineSessionSettings SessionSettings;

	// bIsLANMatch: true면 같은 네트워크에서만 검색 가능
	SessionSettings.bIsLANMatch = false;

	// NumPublicConnections: 최대 참가 인원 (호스트 제외)
	SessionSettings.NumPublicConnections = MaxPlayers;

	// bShouldAdvertise: true면 FindSessions에서 검색됨
	SessionSettings.bShouldAdvertise = true;

	// bUsesPresence: true면 Steam 친구 목록에 "게임 중" 표시
	SessionSettings.bUsesPresence = true;

	// bAllowJoinInProgress: 게임 진행 중 참가 허용 여부
	SessionSettings.bAllowJoinInProgress = false;

	// bAllowJoinViaPresence: Steam 친구가 "참가" 버튼으로 들어올 수 있게
	SessionSettings.bAllowJoinViaPresence = true;

	// bUseLobbiesIfAvailable: Steam Lobby API 사용 (권장)
	SessionSettings.bUseLobbiesIfAvailable = true;

	// 커스텀 데이터: 방 목록에서 이 정보로 필터링/표시
	SessionSettings.Set(TEXT("ROOM_NAME"),
		LocalPlayerName + TEXT("'s Room"),
		EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings.Set(TEXT("HOST_NAME"),
		LocalPlayerName,
		EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	// 콜백 등록: 세션 생성 완료되면 OnCreateSessionComplete 호출됨
	CreateSessionHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(
			this, &UKrakenGameInstance::OnCreateSessionComplete));

	UE_LOG(LogTemp, Log, TEXT("[GameInstance] Creating Steam session. MaxPlayers: %d"), MaxPlayers);
	Sessions->CreateSession(0, NAME_GameSession, SessionSettings);
}

// ============================================================================
// 콜백: 세션 생성 완료
// ============================================================================

void UKrakenGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	// 콜백 정리 (한 번만 호출되도록)
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (Sessions.IsValid())
	{
		Sessions->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionHandle);
	}

	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Log, TEXT("[GameInstance] Steam session created! Traveling to game map."));

		// Listen Server로 게임 맵 이동
		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(GameMapPath + TEXT("?listen"));
		}

		OnSessionOperationComplete.Broadcast(true, TEXT("Room created!"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[GameInstance] Failed to create Steam session!"));
		OnSessionOperationComplete.Broadcast(false, TEXT("Failed to create room."));
	}

	UWorld* World = GetWorld();
	if (World)
	{
		World->ServerTravel(GameMapPath + TEXT("?listen"));
	}

	OnSessionOperationComplete.Broadcast(bWasSuccessful,
		bWasSuccessful ? TEXT("Room created!") : TEXT("Room created (without Steam session)"));
}

// ============================================================================
// 방 검색
// ============================================================================

void UKrakenGameInstance::FindSessions()
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameInstance] No Steam. Cannot search sessions."));
		TArray<FKrakenSessionInfo> EmptyResults;
		OnSessionSearchComplete.Broadcast(EmptyResults);
		return;
	}

	// FOnlineSessionSearch: 검색 조건 설정
	SessionSearch = MakeShareable(new FOnlineSessionSearch());

	// 최대 검색 결과 수
	SessionSearch->MaxSearchResults = 20;

	// LAN 검색인지 인터넷 검색인지
	SessionSearch->bIsLanQuery = false;

	// Presence 기반 검색 (Steam Lobby 검색에 필요)
	SessionSearch->QuerySettings.Set(FName(TEXT("PRESENCESEARCH")), true, EOnlineComparisonOp::Equals);

	// 콜백 등록
	FindSessionsHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateUObject(
			this, &UKrakenGameInstance::OnFindSessionsComplete));

	UE_LOG(LogTemp, Log, TEXT("[GameInstance] Searching for sessions..."));
	Sessions->FindSessions(0, SessionSearch.ToSharedRef());
}

// ============================================================================
// 콜백: 검색 완료
// ============================================================================

void UKrakenGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (Sessions.IsValid())
	{
		Sessions->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsHandle);
	}

	TArray<FKrakenSessionInfo> Results;

	if (bWasSuccessful && SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("[GameInstance] Found %d sessions."),
			   SessionSearch->SearchResults.Num());

		for (int32 i = 0; i < SessionSearch->SearchResults.Num(); i++)
		{
			const FOnlineSessionSearchResult& SearchResult = SessionSearch->SearchResults[i];

			FKrakenSessionInfo Info;
			Info.SearchResultIndex = i;
			Info.PingInMs = SearchResult.PingInMs;
			Info.MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;
			Info.CurrentPlayers = Info.MaxPlayers - SearchResult.Session.NumOpenPublicConnections;

			// 커스텀 데이터 읽기
			SearchResult.Session.SessionSettings.Get(TEXT("ROOM_NAME"), Info.RoomName);
			SearchResult.Session.SessionSettings.Get(TEXT("HOST_NAME"), Info.HostName);

			if (Info.RoomName.IsEmpty())
			{
				Info.RoomName = FString::Printf(TEXT("Room %d"), i + 1);
			}

			Results.Add(Info);

			UE_LOG(LogTemp, Log, TEXT("[GameInstance]   [%d] %s by %s (%d/%d) %dms"),
				   i, *Info.RoomName, *Info.HostName,
				   Info.CurrentPlayers, Info.MaxPlayers, Info.PingInMs);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameInstance] Session search failed or no results."));
	}

	// UI에 결과 전달 (방 목록 갱신)
	OnSessionSearchComplete.Broadcast(Results);
}

// ============================================================================
// 방 참가 (검색 결과에서)
// ============================================================================

void UKrakenGameInstance::JoinSessionByIndex(int32 SessionIndex)
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions.IsValid() || !SessionSearch.IsValid())
	{
		OnSessionOperationComplete.Broadcast(false, TEXT("No session interface."));
		return;
	}

	if (!SessionSearch->SearchResults.IsValidIndex(SessionIndex))
	{
		OnSessionOperationComplete.Broadcast(false, TEXT("Invalid session index."));
		return;
	}

	// 콜백 등록
	JoinSessionHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(
			this, &UKrakenGameInstance::OnJoinSessionComplete));

	UE_LOG(LogTemp, Log, TEXT("[GameInstance] Joining session %d..."), SessionIndex);
	Sessions->JoinSession(0, NAME_GameSession, SessionSearch->SearchResults[SessionIndex]);
}

// ============================================================================
// 콜백: 참가 완료
// ============================================================================

void UKrakenGameInstance::OnJoinSessionComplete(FName SessionName,
	EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (Sessions.IsValid())
	{
		Sessions->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionHandle);
	}

	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG(LogTemp, Log, TEXT("[GameInstance] Joined session successfully!"));

		// GetResolvedConnectString: Steam P2P 주소를 가져옴
		// IP 대신 Steam의 가상 주소를 사용해서 NAT/방화벽을 우회
		FString ConnectInfo;
		if (Sessions.IsValid() && Sessions->GetResolvedConnectString(SessionName, ConnectInfo))
		{
			UE_LOG(LogTemp, Log, TEXT("[GameInstance] Traveling to: %s"), *ConnectInfo);

			APlayerController* PC = GetFirstLocalPlayerController();
			if (PC)
			{
				PC->ClientTravel(ConnectInfo, TRAVEL_Absolute);
			}

			OnSessionOperationComplete.Broadcast(true, TEXT("Joined room!"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[GameInstance] Could not get connect string!"));
			OnSessionOperationComplete.Broadcast(false, TEXT("Failed to get server address."));
		}
	}
	else
	{
		const TCHAR* ResultStr = TEXT("Unknown");
		switch (Result)
		{
		case EOnJoinSessionCompleteResult::SessionIsFull:     ResultStr = TEXT("Room is full"); break;
		case EOnJoinSessionCompleteResult::SessionDoesNotExist: ResultStr = TEXT("Room not found"); break;
		case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress: ResultStr = TEXT("Cannot connect"); break;
		case EOnJoinSessionCompleteResult::AlreadyInSession:  ResultStr = TEXT("Already in a room"); break;
		default: break;
		}

		UE_LOG(LogTemp, Error, TEXT("[GameInstance] Failed to join: %s"), ResultStr);
		OnSessionOperationComplete.Broadcast(false, FString::Printf(TEXT("Join failed: %s"), ResultStr));
	}
}

// ============================================================================
// IP 직접 접속 (LAN/디버그용, Steam 불필요)
// ============================================================================

void UKrakenGameInstance::JoinByIP(const FString& IPAddress, const FString& PlayerName)
{
	LocalPlayerName = PlayerName;

	APlayerController* PC = GetFirstLocalPlayerController();
	if (PC)
	{
		PC->ClientTravel(IPAddress, TRAVEL_Absolute);
		UE_LOG(LogTemp, Log, TEXT("[GameInstance] Joining by IP: %s"), *IPAddress);
	}
}

// ============================================================================
// 세션 나가기
// ============================================================================

void UKrakenGameInstance::LeaveSession()
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (Sessions.IsValid())
	{
		FNamedOnlineSession* ExistingSession = Sessions->GetNamedSession(NAME_GameSession);
		if (ExistingSession)
		{
			UE_LOG(LogTemp, Log, TEXT("[GameInstance] Destroying session before leaving."));
			Sessions->DestroySession(NAME_GameSession);
		}
	}

	GoToMainMenu();
}

// ============================================================================
// 콜백: 세션 파괴 완료
// ============================================================================

void UKrakenGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (Sessions.IsValid())
	{
		Sessions->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionHandle);
	}

	UE_LOG(LogTemp, Log, TEXT("[GameInstance] Session destroyed: %s"),
		   bWasSuccessful ? TEXT("OK") : TEXT("Failed"));

	// HostGame에서 기존 세션 파괴 후 재생성 요청이 있었으면
	if (bPendingCreateAfterDestroy)
	{
		bPendingCreateAfterDestroy = false;
		CreateSessionInternal(PendingMaxPlayers);
	}
}

// ============================================================================
// 메인 메뉴로 이동
// ============================================================================

void UKrakenGameInstance::GoToMainMenu()
{
	APlayerController* PC = GetFirstLocalPlayerController();
	if (PC)
	{
		PC->ClientTravel(MainMenuMapPath, TRAVEL_Absolute);
	}

	UE_LOG(LogTemp, Log, TEXT("[GameInstance] Returning to main menu."));
}
