// KrakenPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "KrakenTypes.h"
#include "KrakenPlayerController.generated.h"


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
	// Server RPC (클라이언트 → 서버)
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
	// Client RPC (서버 → 이 클라이언트만)
	// ========================================================================

	UFUNCTION(Client, Reliable)
	void ClientReceiveCardInfo(int32 EmptyCount, int32 TreasureCount, bool bHasKraken);

	// 매개변수명 주의: AActor에 이미 'Role'이라는 멤버가 있어서
	// shadowing 에러가 발생하므로 반드시 다른 이름을 사용
	UFUNCTION(Client, Reliable)
	void ClientReceiveRole(EPlayerRole InNewRole);

	UFUNCTION(Client, Reliable)
	void ClientReceiveChatMessage(const FString& SenderName, const FString& Message);

	UFUNCTION(Client, Reliable)
	void ClientReceiveNotification(const FString& Message);

	// ========================================================================
	// Multicast RPC (서버 → 모든 클라이언트)
	// ========================================================================

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnBoxRevealed(int32 TargetPlayerIndex, int32 BoxIndex, ECardType CardType);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastChatMessage(const FString& SenderName, const FString& Message);

	// ========================================================================
	// 로컬 클라이언트 상태 (복제되지 않음)
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
