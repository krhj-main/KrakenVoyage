// KrakenPlayerController.cpp

#include "KrakenPlayerController.h"
#include "KrakenGameMode.h"
#include "KrakenPlayerState.h"
#include "KrakenHUDWidget.h"
#include "LobbyWidget.h"

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
