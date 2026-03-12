// KrakenPlayerController.cpp

#include "KrakenPlayerController.h"
#include "KrakenGameMode.h"
#include "KrakenPlayerState.h"
#include "KrakenHUDWidget.h"

AKrakenPlayerController::AKrakenPlayerController()
{
}

void AKrakenPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 로컬 플레이어에게만 HUD 생성 (서버의 다른 플레이어 컨트롤러에서는 생성하지 않음)
	if (IsLocalController())
	{
		CreateHUD();
	}
}

// ============================================================================
// HUD 관리
// ============================================================================

void AKrakenPlayerController::CreateHUD()
{
	if (!HUDWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PC] HUDWidgetClass is not set! Assign it in the Blueprint."));
		return;
	}

	if (HUDWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PC] HUD already exists."));
		return;
	}

	// 위젯 생성 및 뷰포트에 추가
	// CreateWidget<위젯클래스>(소유자, 위젯 블루프린트 클래스)
	HUDWidget = CreateWidget<UKrakenHUDWidget>(this, HUDWidgetClass);
	if (HUDWidget)
	{
		HUDWidget->AddToViewport(0);
		UE_LOG(LogTemp, Log, TEXT("[PC] HUD Widget created and added to viewport."));
	}
}

// ============================================================================
// Server RPC
// ============================================================================

void AKrakenPlayerController::ServerSelectBox_Implementation(int32 TargetPlayerIndex, int32 BoxIndex)
{
	AKrakenGameMode* GM = GetWorld()->GetAuthGameMode<AKrakenGameMode>();
	if (GM)
	{
		GM->HandleBoxSelectionRequest(this, TargetPlayerIndex, BoxIndex);
	}
}

void AKrakenPlayerController::ServerConfirmReveal_Implementation()
{
	AKrakenGameMode* GM = GetWorld()->GetAuthGameMode<AKrakenGameMode>();
	if (GM)
	{
		GM->HandleConfirmReveal(this);
	}
}

void AKrakenPlayerController::ServerSendChatMessage_Implementation(const FString& Message)
{
	const FString SenderName = PlayerState ? PlayerState->GetPlayerName() : TEXT("Unknown");
	UE_LOG(LogTemp, Log, TEXT("[Chat] %s: %s"), *SenderName, *Message);
	MulticastChatMessage(SenderName, Message);
}

void AKrakenPlayerController::ServerToggleReady_Implementation()
{
	AKrakenPlayerState* KPS = GetPlayerState<AKrakenPlayerState>();
	if (KPS)
	{
		KPS->SetReady(!KPS->bIsReady);
		const bool bReady = KPS->bIsReady;
		UE_LOG(LogTemp, Log, TEXT("[Lobby] Player %s ready: %s"), 
			   *KPS->GetPlayerName(), bReady ? TEXT("YES") : TEXT("NO"));
	}
}

void AKrakenPlayerController::ServerRequestStartGame_Implementation()
{
	AKrakenPlayerState* KPS = GetPlayerState<AKrakenPlayerState>();
	if (KPS && KPS->PlayerIndex != 0)
	{
		ClientReceiveNotification(TEXT("Only the host can start the game."));
		return;
	}

	AKrakenGameMode* GM = GetWorld()->GetAuthGameMode<AKrakenGameMode>();
	if (GM)
	{
		GM->StartGame();
	}
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

	UE_LOG(LogTemp, Log, TEXT("[Client] My cards - Empty: %d, Treasure: %d, Kraken: %s"),
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

// ============================================================================
// Multicast RPC
// ============================================================================

void AKrakenPlayerController::MulticastOnBoxRevealed_Implementation(
	int32 TargetPlayerIndex, int32 BoxIndex, ECardType CardType)
{
	UE_LOG(LogTemp, Log, TEXT("[Multicast] Box revealed - Player %d, Box %d, Type: %d"),
		   TargetPlayerIndex, BoxIndex, static_cast<int32>(CardType));
}

void AKrakenPlayerController::MulticastChatMessage_Implementation(
	const FString& SenderName, const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("[MulticastChat] %s: %s"), *SenderName, *Message);
}
