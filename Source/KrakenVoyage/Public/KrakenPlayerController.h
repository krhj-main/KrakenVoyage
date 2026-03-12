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
	// HUD
	// ========================================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UKrakenHUDWidget> HUDWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	UKrakenHUDWidget* HUDWidget = nullptr;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void CreateHUD();

	// ========================================================================
	// 콘솔 명령 (` 키로 콘솔 열고 입력)
	// ========================================================================
	// UFUNCTION(Exec)는 PlayerController에서만 사용 가능
	// 콘솔에서 함수명을 직접 입력하면 실행됨

	// 콘솔: "StartKrakenGame" → 게임 시작
	UFUNCTION(Exec)
	void StartKrakenGame();

	// 콘솔: "ConfirmReveal" → 상자 선택 확정
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

	// 내 플레이어 인덱스 (HUD에서 "내 턴인지" 판단에 사용)
	UFUNCTION(BlueprintCallable, Category = "My Info")
	int32 GetMyPlayerIndex() const;
};
