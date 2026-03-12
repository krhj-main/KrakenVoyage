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

	// 플레이어를 순서대로 다른 Player Start에 스폰
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

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

	// ========================================================================
	// 디버그 모드
	// ========================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDebugMode = true;

	// 디버그: 시뮬레이션할 플레이어 수
	// 실제 접속자보다 크면 나머지는 가상 플레이어
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug",
		meta = (ClampMin = "2", ClampMax = "8", EditCondition = "bDebugMode"))
	int32 DebugPlayerCount = 4;

	// 디버그: BeginPlay에서 자동 게임 시작
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

	// Player Start 액터 목록 (ChoosePlayerStart에서 순서대로 배정)
	TArray<AActor*> PlayerStartActors;

	FTimerHandle DiscussionTimerHandle;
	FTimerHandle PhaseTransitionTimerHandle;

	void OnDiscussionTimeExpired();

	void GetRoleCountsForPlayerCount(int32 PlayerCount, int32& OutCrewCount, int32& OutKrakenCount) const;
	void GetCardCountsForPlayerCount(int32 PlayerCount, int32& OutEmptyCount, int32& OutTreasureCount, int32& OutKrakenCount) const;
	APlayerController* GetPlayerControllerByIndex(int32 Index) const;
	int32 GetRemainingCardCount(int32 PlayerIndex) const;
	int32 GetEffectivePlayerCount() const;

	// 플레이어의 인덱스 가져오기
	int32 GetPlayerIndex(APlayerController* PC) const;
};
