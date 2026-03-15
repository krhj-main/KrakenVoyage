// KrakenPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "KrakenTypes.h"
#include "KrakenPlayerController.generated.h"

class UKrakenHUDWidget;
class ULobbyWidget;

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
	// UI 위젯
	// ========================================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UKrakenHUDWidget> HUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<ULobbyWidget> LobbyWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	UKrakenHUDWidget* HUDWidget = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	ULobbyWidget* LobbyWidget = nullptr;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void CreateHUD();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void CreateLobbyWidget();

	// 로비 → 게임 전환 시 위젯 교체
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowGameHUD();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowLobby();

	// ========================================================================
	// 콘솔 명령
	// ========================================================================

	UFUNCTION(Exec)
	void StartKrakenGame();

	UFUNCTION(Exec)
	void ConfirmReveal();

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

	// 게임이 시작되면 서버가 이 RPC를 호출하여 로비→HUD 전환
	UFUNCTION(Client, Reliable)
	void ClientOnGameStarted();

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

	UFUNCTION(BlueprintCallable, Category = "My Info")
	int32 GetMyPlayerIndex() const;
};
