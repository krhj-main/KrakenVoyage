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

	if (IsLocalController())
	{
		CreateHUD();
	}
}

void AKrakenPlayerController::CreateHUD()
{
	if (!HUDWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PC] HUDWidgetClass is not set!"));
		return;
	}
	if (HUDWidget) return;

	HUDWidget = CreateWidget<UKrakenHUDWidget>(this, HUDWidgetClass);
	if (HUDWidget)
	{
		HUDWidget->AddToViewport(0);
		UE_LOG(LogTemp, Log, TEXT("[PC] HUD created."));
	}
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

// ============================================================================
// GetMyPlayerIndex
// ============================================================================

int32 AKrakenPlayerController::GetMyPlayerIndex() const
{
	AKrakenPlayerState* KPS = GetPlayerState<AKrakenPlayerState>();
	if (KPS)
	{
		return KPS->PlayerIndex;
	}
	return -1;
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
	}
}

void AKrakenPlayerController::ServerRequestStartGame_Implementation()
{
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
