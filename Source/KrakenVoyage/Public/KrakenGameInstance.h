// KrakenGameInstance.h
// ============================================================================
// 게임 인스턴스 - Steam 세션 관리 + 레벨 전환
// ============================================================================
//
// Steam 세션의 비유:
//   Steam 세션 = "식당 예약"
//   CreateSession = 식당에 예약을 걸어둠 (다른 사람이 검색 가능)
//   FindSessions = 예약 가능한 식당 목록 조회
//   JoinSession = 예약된 식당에 합류
//   DestroySession = 예약 취소
//
// 모든 Steam 작업은 비동기:
//   요청() → 기다림... → 콜백(결과) 패턴
//
// Steam 없이도 동작:
//   Steam이 없으면 자동으로 기존 IP 직접 연결 방식으로 폴백
//
// ============================================================================

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "KrakenGameInstance.generated.h"

// 방 검색 결과 구조체 (UI에서 방 목록 표시용)
USTRUCT(BlueprintType)
struct FKrakenSessionInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString RoomName;

	UPROPERTY(BlueprintReadOnly)
	FString HostName;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentPlayers = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 MaxPlayers = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 PingInMs = 0;

	// SearchResults 배열의 인덱스 (JoinSessionByIndex에서 사용)
	int32 SearchResultIndex = -1;
};

// 방 검색 완료 이벤트 (UI 바인딩용)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionSearchComplete,
	const TArray<FKrakenSessionInfo>&, Results);

// 세션 작업 완료 이벤트 (성공/실패 메시지)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSessionOperationComplete,
	bool, bSuccess, const FString&, Message);


UCLASS()
class KRAKENVOYAGE_API UKrakenGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UKrakenGameInstance();

	// Init: 엔진 초기화 시 호출 (Steam 연결 상태 로그)
	virtual void Init() override;

	// ========================================================================
	// 세션 관리 (Steam 있으면 Steam, 없으면 IP 직접 연결)
	// ========================================================================

	// 방 만들기 (Host)
	UFUNCTION(BlueprintCallable, Category = "Session")
	void HostGame(const FString& PlayerName, int32 MaxPlayers = 6);

	// 방 목록 검색
	UFUNCTION(BlueprintCallable, Category = "Session")
	void FindSessions();

	// 검색 결과에서 선택하여 참가
	UFUNCTION(BlueprintCallable, Category = "Session")
	void JoinSessionByIndex(int32 SessionIndex);

	// IP 직접 접속 (LAN/디버그용)
	UFUNCTION(BlueprintCallable, Category = "Session")
	void JoinByIP(const FString& IPAddress, const FString& PlayerName);

	// 현재 세션 나가기
	UFUNCTION(BlueprintCallable, Category = "Session")
	void LeaveSession();

	// 메인 메뉴로 이동
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void GoToMainMenu();

	// ========================================================================
	// 이벤트 (블루프린트/UI에서 바인딩)
	// ========================================================================

	// 방 검색 완료 시 발생 → 방 목록 UI 갱신
	UPROPERTY(BlueprintAssignable, Category = "Session Events")
	FOnSessionSearchComplete OnSessionSearchComplete;

	// 세션 작업 완료 시 발생 → 상태 메시지 표시
	UPROPERTY(BlueprintAssignable, Category = "Session Events")
	FOnSessionOperationComplete OnSessionOperationComplete;

	// ========================================================================
	// 플레이어 / 맵 정보
	// ========================================================================

	UPROPERTY(BlueprintReadWrite, Category = "Player Info")
	FString LocalPlayerName = TEXT("Player");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maps")
	FString MainMenuMapPath = TEXT("/Game/Maps/L_MainMenu");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maps")
	FString GameMapPath = TEXT("/Game/Maps/L_Shop_Prototype");

protected:
	// ========================================================================
	// Steam 세션 콜백 (비동기 결과 수신)
	// ========================================================================

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	// ========================================================================
	// 내부 헬퍼
	// ========================================================================

	// Online Session Interface 가져오기 (Steam 없으면 nullptr)
	IOnlineSessionPtr GetSessionInterface() const;

	// 기존 세션 정리 후 새 세션 생성
	void CreateSessionInternal(int32 MaxPlayers);

	// ========================================================================
	// 내부 상태
	// ========================================================================

	// 검색 결과 저장 (JoinSessionByIndex에서 참조)
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	// 세션 파괴 후 재생성이 필요한 경우
	bool bPendingCreateAfterDestroy = false;
	int32 PendingMaxPlayers = 6;

	// 델리게이트 핸들 (콜백 정리용)
	FDelegateHandle CreateSessionHandle;
	FDelegateHandle FindSessionsHandle;
	FDelegateHandle JoinSessionHandle;
	FDelegateHandle DestroySessionHandle;
};
