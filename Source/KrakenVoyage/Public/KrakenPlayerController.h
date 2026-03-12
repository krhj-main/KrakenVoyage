// KrakenPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "KrakenTypes.h"
#include "KrakenPlayerController.generated.h"

class UKrakenHUDWidget;

UCLASS()
class KRAKENVOYAGE_API AKrakenPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AKrakenPlayerController();

protected:
	virtual void BeginPlay() override;

public:

	// ========================================================================
	// HUD 위젯
	// ========================================================================

	// 블루프린트에서 만든 위젯 클래스를 여기에 할당
	// BP_KrakenPlayerController의 디테일에서 WBP_HUD를 지정
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UKrakenHUDWidget> HUDWidgetClass;

	// 생성된 위젯 인스턴스
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	UKrakenHUDWidget* HUDWidget = nullptr;

	// HUD 생성 및 표시
	UFUNCTION(BlueprintCallable, Category = "UI")
	void CreateHUD();

	// ========================================================================
	// Server RPC
	// ========================================================================

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Game Actions")
	void ServerSelectBox(int32 TargetPlayerIndex, int32 BoxIndex);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Game Actions")
	void ServerConfirmReveal();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Chat")
	void ServerSendChatMessage(const FString& Message);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Lobby")
	void ServerToggleReady();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Lobby")
	void ServerRequestStartGame();

	// ========================================================================
	// Client RPC
	// ========================================================================

	UFUNCTION(Client, Reliable)
	void ClientReceiveCardInfo(int32 EmptyCount, int32 TreasureCount, bool bHasKraken);

	UFUNCTION(Client, Reliable)
	void ClientReceiveRole(EPlayerRole InNewRole);

	UFUNCTION(Client, Reliable)
	void ClientReceiveChatMessage(const FString& SenderName, const FString& Message);

	UFUNCTION(Client, Reliable)
	void ClientReceiveNotification(const FString& Message);

	// ========================================================================
	// Multicast RPC
	// ========================================================================

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnBoxRevealed(int32 TargetPlayerIndex, int32 BoxIndex, ECardType CardType);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastChatMessage(const FString& SenderName, const FString& Message);

	// ========================================================================
	// 로컬 상태
	// ========================================================================

	UPROPERTY(BlueprintReadOnly, Category = "My Cards")
	int32 MyEmptyCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "My Cards")
	int32 MyTreasureCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "My Cards")
	bool bMyHasKraken = false;

	UPROPERTY(BlueprintReadOnly, Category = "My Info")
	EPlayerRole MyRole = EPlayerRole::None;
};
