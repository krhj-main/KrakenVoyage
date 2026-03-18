// KrakenPlayerController.cpp

#include "KrakenPlayerController.h"
#include "KrakenGameMode.h"
#include "KrakenPlayerState.h"
#include "KrakenHUDWidget.h"
#include "LobbyWidget.h"
#include "GameOverWidget.h"

AKrakenPlayerController::AKrakenPlayerController()
{
}

void AKrakenPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		// 디버그 모드 자동시작이면 HUD 바로 표시
		// 아니면 로비 위젯 먼저 표시
		AKrakenGameMode* GM = GetWorld()->GetAuthGameMode<AKrakenGameMode>();
		const bool bAutoStart = GM && GM->bDebugMode && GM->bAutoStartGame;

		if (bAutoStart)
		{
			CreateHUD();
		}
		else
		{
			CreateLobbyWidget();
		}
	}
}

// ============================================================================
// UI 관리
// ============================================================================

void AKrakenPlayerController::CreateHUD()
{
	if (!HUDWidgetClass || HUDWidget) return;

	HUDWidget = CreateWidget<UKrakenHUDWidget>(this, HUDWidgetClass);
	if (HUDWidget)
	{
		HUDWidget->AddToViewport(0);
		UE_LOG(LogTemp, Log, TEXT("[PC] HUD created."));
	}
}

void AKrakenPlayerController::CreateLobbyWidget()
{
	if (!LobbyWidgetClass || LobbyWidget) return;

	LobbyWidget = CreateWidget<ULobbyWidget>(this, LobbyWidgetClass);
	if (LobbyWidget)
	{
		LobbyWidget->AddToViewport(0);

		// 로비에서는 마우스 커서 보이게 + UI 클릭 가능
		bShowMouseCursor = true;
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);

		UE_LOG(LogTemp, Log, TEXT("[PC] Lobby widget created."));
	}
}

void AKrakenPlayerController::ShowGameHUD()
{
	// 로비 숨기고 HUD 표시
	if (LobbyWidget)
	{
		LobbyWidget->RemoveFromParent();
		LobbyWidget = nullptr;
	}

	// 게임 중에는 마우스 숨기고 게임 입력 모드
	bShowMouseCursor = false;
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	CreateHUD();
}

void AKrakenPlayerController::ShowLobby()
{
	// HUD 숨기고 로비 표시
	if (HUDWidget)
	{
		HUDWidget->RemoveFromParent();
		HUDWidget = nullptr;
	}

	CreateLobbyWidget();
}

// ============================================================================
// 콘솔 명령
// ============================================================================

void AKrakenPlayerController::StartKrakenGame()
{
	UE_LOG(LogTemp, Log, TEXT("[PC] Console: StartKrakenGame"));
	ServerRequestStartGame();
}

void AKrakenPlayerController::ConfirmReveal()
{
	UE_LOG(LogTemp, Log, TEXT("[PC] Console: ConfirmReveal"));
	ServerConfirmReveal();
}

int32 AKrakenPlayerController::GetMyPlayerIndex() const
{
	AKrakenPlayerState* KPS = GetPlayerState<AKrakenPlayerState>();
	return KPS ? KPS->PlayerIndex : -1;
}

// ============================================================================
// Server RPC
// ============================================================================

void AKrakenPlayerController::ServerSelectBox_Implementation(int32 TargetPlayerIndex, int32 BoxIndex)
{
	AKrakenGameMode* GM = GetWorld()->GetAuthGameMode<AKrakenGameMode>();
	if (GM) GM->HandleBoxSelectionRequest(this, TargetPlayerIndex, BoxIndex);
}

void AKrakenPlayerController::ServerConfirmReveal_Implementation()
{
	AKrakenGameMode* GM = GetWorld()->GetAuthGameMode<AKrakenGameMode>();
	if (GM) GM->HandleConfirmReveal(this);
}

void AKrakenPlayerController::ServerSendChatMessage_Implementation(const FString& Message)
{
	const FString SenderName = PlayerState ? PlayerState->GetPlayerName() : TEXT("Unknown");
	MulticastChatMessage(SenderName, Message);
}

void AKrakenPlayerController::ServerToggleReady_Implementation()
{
	AKrakenPlayerState* KPS = GetPlayerState<AKrakenPlayerState>();
	if (KPS) KPS->SetReady(!KPS->bIsReady);
}

void AKrakenPlayerController::ServerRequestStartGame_Implementation()
{
	AKrakenGameMode* GM = GetWorld()->GetAuthGameMode<AKrakenGameMode>();
	if (GM) GM->StartGame();
}

// ============================================================================
// Client RPC
// ============================================================================

void AKrakenPlayerController::ClientReceiveCardInfo_Implementation(
	int32 EmptyCount, int32 TreasureCount, bool bHasKraken)
{
	MyEmptyCount = EmptyCount;
	MyTreasureCount = TreasureCount;
	bMyHasKraken = bHasKraken;
	UE_LOG(LogTemp, Log, TEXT("[Client] Cards - Empty:%d Treasure:%d Kraken:%s"),
		   EmptyCount, TreasureCount, bHasKraken ? TEXT("YES") : TEXT("NO"));
}

void AKrakenPlayerController::ClientReceiveRole_Implementation(EPlayerRole InNewRole)
{
	MyRole = InNewRole;
	const TCHAR* RoleName = (InNewRole == EPlayerRole::Crew) ? TEXT("CREW") : TEXT("KRAKEN");
	UE_LOG(LogTemp, Log, TEXT("[Client] My role: %s"), RoleName);
}

void AKrakenPlayerController::ClientReceiveChatMessage_Implementation(
	const FString& SenderName, const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("[Chat] %s: %s"), *SenderName, *Message);
}

void AKrakenPlayerController::ClientReceiveNotification_Implementation(const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("[Notification] %s"), *Message);
}

void AKrakenPlayerController::ClientOnGameStarted_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("[Client] Game started! Switching to HUD."));
	ShowGameHUD();
}

// ============================================================================
// Multicast RPC
// ============================================================================

void AKrakenPlayerController::MulticastOnBoxRevealed_Implementation(
	int32 TargetPlayerIndex, int32 BoxIndex, ECardType CardType)
{
	UE_LOG(LogTemp, Log, TEXT("[Multicast] Box revealed - Player %d, Box %d"), 
		   TargetPlayerIndex, BoxIndex);
}

void AKrakenPlayerController::MulticastChatMessage_Implementation(
	const FString& SenderName, const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("[MulticastChat] %s: %s"), *SenderName, *Message);
}

void AKrakenPlayerController::ServerSetTalking_Implementation(bool bNewTalking)
{
	AKrakenPlayerState* PS = GetPlayerState<AKrakenPlayerState>();
	if (PS)
	{
		PS->bIsTalking = bNewTalking;
	}
}

// ============================================================
// ★ 게임오버 UI 표시
// ============================================================
void AKrakenPlayerController::ClientShowGameOver_Implementation(
	EWinCondition InWinResult, bool bPlayerWon,
	const FString& PlayerRolesText,
	int32 TreasureFound, int32 TreasureTotal,
	int32 RoundsPlayed, int32 MaxRounds,
	int32 CardsRevealed)
{
	// 기존 HUD의 GameResult 텍스트 대신 전체 화면 오버레이 사용

	if (!GameOverWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PC] GameOverWidgetClass not set!"));
		return;
	}

	// 이미 표시 중이면 무시
	if (GameOverWidget && GameOverWidget->IsInViewport())
	{
		return;
	}

	GameOverWidget = CreateWidget<UGameOverWidget>(this, GameOverWidgetClass);
	if (GameOverWidget)
	{
		GameOverWidget->AddToViewport(20); // HUD(10) 위에 표시
		GameOverWidget->SetupResult(
			InWinResult, bPlayerWon,
			PlayerRolesText,
			TreasureFound, TreasureTotal,
			RoundsPlayed, MaxRounds,
			CardsRevealed);

		// 마우스 커서 활성화 (버튼 클릭 가능하도록)
		// SetShowMouseCursor: PlayerController의 마우스 표시 설정
		SetShowMouseCursor(true);

		// FInputModeGameAndUI: 게임 + UI 모두 입력 받는 모드
		// 캐릭터 이동도 되고 버튼 클릭도 됨
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);

		UE_LOG(LogTemp, Log, TEXT("[PC] GameOver widget shown. Won: %s"),
			bPlayerWon ? TEXT("YES") : TEXT("NO"));
	}
}