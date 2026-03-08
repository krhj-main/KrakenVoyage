// KrakenGameMode.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "KrakenTypes.h"
#include "KrakenGameMode.generated.h"

class AExplorationBox;

UCLASS()
class KRAKENVOYAGE_API AKrakenGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AKrakenGameMode();

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void StartGame();

	void TransitionToPhase(EKrakenGamePhase NewPhase);

	void BeginRoleRevealPhase();
	void BeginCardCheckPhase();
	void BeginDiscussionPhase();
	void BeginRevealPhase();
	void BeginRoundTransitionPhase();
	void BeginGameOverPhase(EWinCondition WinResult);

	void AssignRoles();
	void CreateAndShuffleCards();
	void DistributeCards();
	void CollectAndRedistributeCards();

	void HandleBoxSelectionRequest(APlayerController* RequestingPlayer, 
								   int32 TargetPlayerIndex, int32 BoxIndex);
	void HandleConfirmReveal(APlayerController* RequestingPlayer);
	bool IsCurrentActionHolder(APlayerController* Player) const;
	EWinCondition CheckWinCondition() const;

	void FindAndRegisterBoxes();
	AExplorationBox* FindBox(int32 InPlayerIndex, int32 InBoxIndex) const;
	void ResetAllBoxes();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Settings")
	FRoomSettings RoomSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDebugMode = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug",
		meta = (ClampMin = "4", ClampMax = "8", EditCondition = "bDebugMode"))
	int32 DebugPlayerCount = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug",
		meta = (EditCondition = "bDebugMode"))
	bool bAutoStartGame = true;

protected:

	TArray<ECardType> CardDeck;
	TArray<TArray<ECardType>> PlayerCards;
	TArray<TArray<bool>> PlayerCardRevealed;

	int32 CurrentActionHolderIndex = 0;
	int32 CurrentRound = 1;
	int32 CurrentTurnInRound = 0;
	int32 RevealedTreasureCount = 0;
	int32 TotalTreasureCount = 0;

	TArray<APlayerController*> PlayerControllers;

	int32 PendingTargetPlayerIndex = -1;
	int32 PendingBoxIndex = -1;

	TArray<TArray<AExplorationBox*>> RegisteredBoxes;

	FTimerHandle DiscussionTimerHandle;
	FTimerHandle PhaseTransitionTimerHandle;

	void OnDiscussionTimeExpired();

	// 플레이어 수에 따른 역할 수 (출력 매개변수 방식)
	// FIntVector가 UE5.7에서 변경되어 호환성 문제 발생 → 출력 매개변수로 대체
	void GetRoleCountsForPlayerCount(int32 PlayerCount, int32& OutCrewCount, int32& OutKrakenCount) const;

	// 플레이어 수에 따른 카드 구성 (출력 매개변수 방식)
	void GetCardCountsForPlayerCount(int32 PlayerCount, int32& OutEmptyCount, int32& OutTreasureCount, int32& OutKrakenCount) const;

	APlayerController* GetPlayerControllerByIndex(int32 Index) const;
	int32 GetRemainingCardCount(int32 PlayerIndex) const;
	int32 GetEffectivePlayerCount() const;
};
