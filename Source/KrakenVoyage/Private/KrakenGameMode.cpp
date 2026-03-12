// KrakenGameMode.cpp

#include "KrakenGameMode.h"
#include "KrakenGameState.h"
#include "KrakenPlayerState.h"
#include "KrakenPlayerController.h"
#include "KrakenCharacter.h"
#include "ExplorationBox.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"

AKrakenGameMode::AKrakenGameMode()
{
	GameStateClass = AKrakenGameState::StaticClass();
	PlayerStateClass = AKrakenPlayerState::StaticClass();
	PlayerControllerClass = AKrakenPlayerController::StaticClass();
	DefaultPawnClass = AKrakenCharacter::StaticClass();
}

void AKrakenGameMode::BeginPlay()
{
	Super::BeginPlay();

	FindAndRegisterBoxes();

	if (bDebugMode && bAutoStartGame)
	{
		// 멀티플레이어일 수 있으니 충분한 딜레이 (모든 플레이어 접속 대기)
		const float StartDelay = 2.0f;
		GetWorldTimerManager().SetTimer(PhaseTransitionTimerHandle, [this]()
		{
			UE_LOG(LogTemp, Log, TEXT("[GameMode] Auto-starting game. Real players: %d"),
				   PlayerControllers.Num());
			StartGame();
		}, StartDelay, false);
	}
}

// ============================================================================
// ChoosePlayerStart - 플레이어를 순서대로 다른 위치에 스폰
// ============================================================================
AActor* AKrakenGameMode::ChoosePlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
	// 레벨의 모든 PlayerStart를 찾아서 캐싱
	if (PlayerStartActors.Num() == 0)
	{
		// PlayerStart가 없으면 기본 동작
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStartActors);
		UE_LOG(LogTemp, Log, TEXT("[GameMode] Found %d PlayerStart actors"), PlayerStartActors.Num());
	}

	if (PlayerStartActors.Num() == 0)
	{
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	// 현재 접속한 플레이어 수 기준으로 순서대로 배정
	// PostLogin이 호출되기 전이므로, 현재 PlayerControllers.Num()이 이 플레이어의 인덱스
	const int32 SpawnIndex = PlayerControllers.Num(); // 아직 Add 전이므로 0부터 시작
	const int32 StartIndex = SpawnIndex % PlayerStartActors.Num();

	UE_LOG(LogTemp, Log, TEXT("[GameMode] Player %d -> PlayerStart %d"), SpawnIndex, StartIndex);
	return PlayerStartActors[StartIndex];
	
}

// ============================================================================
// 상자 관리
// ============================================================================

void AKrakenGameMode::FindAndRegisterBoxes()
{
	TArray<AActor*> FoundBoxes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExplorationBox::StaticClass(), FoundBoxes);

	UE_LOG(LogTemp, Log, TEXT("[GameMode] Found %d ExplorationBox actors in level"), FoundBoxes.Num());

	RegisteredBoxes.Empty();
	RegisteredBoxes.SetNum(8);
	for (int32 i = 0; i < 8; i++)
	{
		RegisteredBoxes[i].SetNum(5);
		for (int32 j = 0; j < 5; j++)
		{
			RegisteredBoxes[i][j] = nullptr;
		}
	}

	for (AActor* Actor : FoundBoxes)
	{
		AExplorationBox* Box = Cast<AExplorationBox>(Actor);
		if (!Box) continue;

		const int32 OwnerIdx = Box->OwnerPlayerIndex;
		const int32 BoxIdx = Box->BoxIndex;

		if (OwnerIdx >= 0 && OwnerIdx < 8 && BoxIdx >= 0 && BoxIdx < 5)
		{
			if (RegisteredBoxes[OwnerIdx][BoxIdx] != nullptr)
			{
				UE_LOG(LogTemp, Warning,
					TEXT("[GameMode] Duplicate box! Player %d, Box %d"), OwnerIdx, BoxIdx);
			}
			RegisteredBoxes[OwnerIdx][BoxIdx] = Box;
			UE_LOG(LogTemp, Log, TEXT("[GameMode] Registered box: Player %d, Box %d -> %s"),
				   OwnerIdx, BoxIdx, *Box->GetName());
		}
	}
}

AExplorationBox* AKrakenGameMode::FindBox(int32 InPlayerIndex, int32 InBoxIndex) const
{
	if (RegisteredBoxes.IsValidIndex(InPlayerIndex) &&
		RegisteredBoxes[InPlayerIndex].IsValidIndex(InBoxIndex))
	{
		return RegisteredBoxes[InPlayerIndex][InBoxIndex];
	}
	return nullptr;
}

void AKrakenGameMode::ResetAllBoxes()
{
	for (int32 p = 0; p < RegisteredBoxes.Num(); p++)
	{
		for (int32 b = 0; b < RegisteredBoxes[p].Num(); b++)
		{
			if (RegisteredBoxes[p][b])
			{
				RegisteredBoxes[p][b]->ResetBox();
			}
		}
	}
}

int32 AKrakenGameMode::GetEffectivePlayerCount() const
{
	if (bDebugMode)
	{
		// 실제 접속자가 DebugPlayerCount 이상이면 실제 수를 사용
		return FMath::Max(DebugPlayerCount, PlayerControllers.Num());
	}
	return PlayerControllers.Num();
}

int32 AKrakenGameMode::GetPlayerIndex(APlayerController* PC) const
{
	return PlayerControllers.IndexOfByKey(PC);
}

// ============================================================================
// 플레이어 접속/퇴장
// ============================================================================

void AKrakenGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	PlayerControllers.Add(NewPlayer);

	AKrakenPlayerState* PS = NewPlayer->GetPlayerState<AKrakenPlayerState>();
	if (PS)
	{
		const int32 NewIndex = PlayerControllers.Num() - 1;
		PS->PlayerIndex = NewIndex;
		UE_LOG(LogTemp, Log, TEXT("[GameMode] Player logged in. Total: %d, Index: %d"),
			   PlayerControllers.Num(), NewIndex);
	}
}

void AKrakenGameMode::Logout(AController* Exiting)
{
	APlayerController* PC = Cast<APlayerController>(Exiting);
	if (PC)
	{
		PlayerControllers.Remove(PC);
	}
	Super::Logout(Exiting);
}

// ============================================================================
// 콘솔 명령
// ============================================================================

// PlayerController의 Exec에서 호출됨
// (GameMode에 직접 Exec를 붙일 수 없으므로 PC를 경유)

// ============================================================================
// 게임 흐름
// ============================================================================

void AKrakenGameMode::StartGame()
{
	const int32 PlayerCount = GetEffectivePlayerCount();

	// 최소 인원 체크 (디버그 모드가 아닐 때)
	if (!bDebugMode && PlayerCount < 4)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] Not enough players. Need 4, have %d"), PlayerCount);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[GameMode] ===== GAME START ===== EffectivePlayers: %d, RealPlayers: %d, Debug: %s"),
		   PlayerCount, PlayerControllers.Num(), bDebugMode ? TEXT("ON") : TEXT("OFF"));

	TotalTreasureCount = PlayerCount;
	RevealedTreasureCount = 0;
	CurrentRound = 1;
	CurrentTurnInRound = 0;
	CurrentActionHolderIndex = 0;

	AssignRoles();
	CreateAndShuffleCards();
	DistributeCards();
	TransitionToPhase(EKrakenGamePhase::RoleReveal);
}

void AKrakenGameMode::TransitionToPhase(EKrakenGamePhase NewPhase)
{
	AKrakenGameState* KrakenGS = GetGameState<AKrakenGameState>();
	if (KrakenGS)
	{
		KrakenGS->SetCurrentPhase(NewPhase);
	}

	switch (NewPhase)
	{
	case EKrakenGamePhase::RoleReveal:      BeginRoleRevealPhase(); break;
	case EKrakenGamePhase::CardCheck:       BeginCardCheckPhase(); break;
	case EKrakenGamePhase::Discussion:      BeginDiscussionPhase(); break;
	case EKrakenGamePhase::Reveal:          BeginRevealPhase(); break;
	case EKrakenGamePhase::RoundTransition: BeginRoundTransitionPhase(); break;
	case EKrakenGamePhase::GameOver:        break;
	default: break;
	}
}

// ============================================================================
// 각 페이즈
// ============================================================================

void AKrakenGameMode::BeginRoleRevealPhase()
{
	UE_LOG(LogTemp, Log, TEXT("[GameMode] Phase: Role Reveal"));

	for (int32 i = 0; i < PlayerControllers.Num(); i++)
	{
		AKrakenPlayerController* KPC = Cast<AKrakenPlayerController>(PlayerControllers[i]);
		AKrakenPlayerState* PS = PlayerControllers[i]->GetPlayerState<AKrakenPlayerState>();
		if (KPC && PS)
		{
			KPC->ClientReceiveRole(PS->PlayerRole);
		}
	}

	GetWorldTimerManager().SetTimer(PhaseTransitionTimerHandle, [this]()
	{
		TransitionToPhase(EKrakenGamePhase::CardCheck);
	}, 3.0f, false);
}

void AKrakenGameMode::BeginCardCheckPhase()
{
	UE_LOG(LogTemp, Log, TEXT("[GameMode] Phase: Card Check"));

	for (int32 i = 0; i < PlayerControllers.Num(); i++)
	{
		AKrakenPlayerController* KPC = Cast<AKrakenPlayerController>(PlayerControllers[i]);
		if (KPC && PlayerCards.IsValidIndex(i))
		{
			int32 EmptyCount = 0;
			int32 TreasureCount = 0;
			bool bHasKraken = false;

			for (const ECardType& Card : PlayerCards[i])
			{
				if (Card == ECardType::Empty)         EmptyCount++;
				else if (Card == ECardType::Treasure)  TreasureCount++;
				else if (Card == ECardType::Kraken)    bHasKraken = true;
			}

			KPC->ClientReceiveCardInfo(EmptyCount, TreasureCount, bHasKraken);
		}
	}

	if (bDebugMode)
	{
		for (int32 i = 0; i < GetEffectivePlayerCount(); i++)
		{
			if (PlayerCards.IsValidIndex(i))
			{
				FString CardStr;
				for (const ECardType& Card : PlayerCards[i])
				{
					if (Card == ECardType::Empty) CardStr += TEXT("E ");
					else if (Card == ECardType::Treasure) CardStr += TEXT("T ");
					else if (Card == ECardType::Kraken) CardStr += TEXT("K ");
				}
				UE_LOG(LogTemp, Log, TEXT("[Debug] Player %d cards: %s"), i, *CardStr);
			}
		}
	}

	GetWorldTimerManager().SetTimer(PhaseTransitionTimerHandle, [this]()
	{
		TransitionToPhase(EKrakenGamePhase::Discussion);
	}, 5.0f, false);
}

void AKrakenGameMode::BeginDiscussionPhase()
{
	UE_LOG(LogTemp, Log, TEXT("[GameMode] Phase: Discussion (Round %d, Turn %d/%d, ActionHolder: %d)"),
		   CurrentRound, CurrentTurnInRound + 1, GetEffectivePlayerCount(), CurrentActionHolderIndex);

	AKrakenGameState* KrakenGS = GetGameState<AKrakenGameState>();
	if (KrakenGS)
	{
		KrakenGS->SetTurnInfo(CurrentRound, CurrentTurnInRound, CurrentActionHolderIndex);
	}

	GetWorldTimerManager().SetTimer(DiscussionTimerHandle,
		this, &AKrakenGameMode::OnDiscussionTimeExpired,
		static_cast<float>(RoomSettings.DiscussionTimeSeconds), false);
}

void AKrakenGameMode::BeginRevealPhase()
{
	UE_LOG(LogTemp, Log, TEXT("[GameMode] Phase: Reveal (Target: Player %d, Box %d)"),
		   PendingTargetPlayerIndex, PendingBoxIndex);

	if (PendingTargetPlayerIndex < 0 || PendingBoxIndex < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] No valid selection."));
		TransitionToPhase(EKrakenGamePhase::Discussion);
		return;
	}

	if (!PlayerCards.IsValidIndex(PendingTargetPlayerIndex) ||
		!PlayerCards[PendingTargetPlayerIndex].IsValidIndex(PendingBoxIndex))
	{
		UE_LOG(LogTemp, Error, TEXT("[GameMode] Invalid card indices!"));
		TransitionToPhase(EKrakenGamePhase::Discussion);
		return;
	}

	const ECardType RevealedCard = PlayerCards[PendingTargetPlayerIndex][PendingBoxIndex];
	PlayerCardRevealed[PendingTargetPlayerIndex][PendingBoxIndex] = true;

	const TCHAR* CardName = TEXT("Empty");
	if (RevealedCard == ECardType::Treasure) CardName = TEXT("TREASURE");
	else if (RevealedCard == ECardType::Kraken) CardName = TEXT("KRAKEN!");
	UE_LOG(LogTemp, Log, TEXT("[GameMode] >>> Card Revealed: %s <<<"), CardName);

	if (RevealedCard == ECardType::Treasure)
	{
		RevealedTreasureCount++;
	}

	AExplorationBox* Box = FindBox(PendingTargetPlayerIndex, PendingBoxIndex);
	if (Box)
	{
		Box->RevealBox(RevealedCard);
	}

	FRevealedCardInfo RevealInfo;
	RevealInfo.RevealerPlayerIndex = CurrentActionHolderIndex;
	RevealInfo.TargetPlayerIndex = PendingTargetPlayerIndex;
	RevealInfo.CardType = RevealedCard;
	RevealInfo.Round = CurrentRound;
	RevealInfo.TurnInRound = CurrentTurnInRound;

	AKrakenGameState* KrakenGS = GetGameState<AKrakenGameState>();
	if (KrakenGS)
	{
		KrakenGS->AddRevealedCard(RevealInfo);
	}

	const EWinCondition WinResult = CheckWinCondition();
	if (WinResult != EWinCondition::None)
	{
		BeginGameOverPhase(WinResult);
		return;
	}

	CurrentActionHolderIndex = PendingTargetPlayerIndex;
	CurrentTurnInRound++;
	PendingTargetPlayerIndex = -1;
	PendingBoxIndex = -1;

	const int32 PlayerCount = GetEffectivePlayerCount();
	if (CurrentTurnInRound >= PlayerCount)
	{
		TransitionToPhase(EKrakenGamePhase::RoundTransition);
	}
	else
	{
		GetWorldTimerManager().SetTimer(PhaseTransitionTimerHandle, [this]()
		{
			TransitionToPhase(EKrakenGamePhase::Discussion);
		}, 2.0f, false);
	}
}

void AKrakenGameMode::BeginRoundTransitionPhase()
{
	UE_LOG(LogTemp, Log, TEXT("[GameMode] Phase: Round Transition (%d -> %d)"),
		   CurrentRound, CurrentRound + 1);

	CurrentRound++;
	CurrentTurnInRound = 0;

	if (CurrentRound > RoomSettings.MaxRounds)
	{
		BeginGameOverPhase(EWinCondition::TimeRanOut);
		return;
	}

	ResetAllBoxes();
	CollectAndRedistributeCards();

	GetWorldTimerManager().SetTimer(PhaseTransitionTimerHandle, [this]()
	{
		TransitionToPhase(EKrakenGamePhase::CardCheck);
	}, 3.0f, false);
}

void AKrakenGameMode::BeginGameOverPhase(EWinCondition WinResult)
{
	const TCHAR* WinName = TEXT("Unknown");
	if (WinResult == EWinCondition::CrewFoundAllTreasure) WinName = TEXT("Crew Found All Treasure!");
	else if (WinResult == EWinCondition::KrakenRevealed) WinName = TEXT("Kraken Revealed!");
	else if (WinResult == EWinCondition::TimeRanOut) WinName = TEXT("Time Ran Out!");

	UE_LOG(LogTemp, Log, TEXT("[GameMode] ===== GAME OVER: %s ====="), WinName);

	AKrakenGameState* KrakenGS = GetGameState<AKrakenGameState>();
	if (KrakenGS)
	{
		KrakenGS->SetCurrentPhase(EKrakenGamePhase::GameOver);
		KrakenGS->SetWinCondition(WinResult);
	}

	for (int32 i = 0; i < PlayerControllers.Num(); i++)
	{
		AKrakenPlayerState* PS = PlayerControllers[i]->GetPlayerState<AKrakenPlayerState>();
		if (PS)
		{
			PS->RevealRole();
		}
	}

	if (bDebugMode)
	{
		for (int32 p = 0; p < PlayerCards.Num(); p++)
		{
			for (int32 b = 0; b < PlayerCards[p].Num(); b++)
			{
				if (!PlayerCardRevealed[p][b])
				{
					AExplorationBox* Box = FindBox(p, b);
					if (Box) Box->RevealBox(PlayerCards[p][b]);
				}
			}
		}
	}
}

// ============================================================================
// 역할 및 카드 배분
// ============================================================================

void AKrakenGameMode::AssignRoles()
{
	const int32 PlayerCount = GetEffectivePlayerCount();
	int32 CrewCount = 0;
	int32 KrakenCount = 0;
	GetRoleCountsForPlayerCount(PlayerCount, CrewCount, KrakenCount);

	TArray<EPlayerRole> Roles;
	for (int32 i = 0; i < CrewCount; i++) Roles.Add(EPlayerRole::Crew);
	for (int32 i = 0; i < KrakenCount; i++) Roles.Add(EPlayerRole::Kraken);

	for (int32 i = Roles.Num() - 1; i > 0; i--)
	{
		Roles.Swap(i, FMath::RandRange(0, i));
	}

	for (int32 i = 0; i < PlayerControllers.Num(); i++)
	{
		AKrakenPlayerState* PS = PlayerControllers[i]->GetPlayerState<AKrakenPlayerState>();
		if (PS && Roles.IsValidIndex(i))
		{
			PS->SetRole(Roles[i]);
			const TCHAR* RoleName = (Roles[i] == EPlayerRole::Crew) ? TEXT("Crew") : TEXT("Kraken");
			UE_LOG(LogTemp, Log, TEXT("[GameMode] Player %d role: %s"), i, RoleName);
		}
	}

	if (bDebugMode)
	{
		for (int32 i = PlayerControllers.Num(); i < PlayerCount; i++)
		{
			if (Roles.IsValidIndex(i))
			{
				const TCHAR* RoleName = (Roles[i] == EPlayerRole::Crew) ? TEXT("Crew") : TEXT("Kraken");
				UE_LOG(LogTemp, Log, TEXT("[Debug] Virtual Player %d role: %s"), i, RoleName);
			}
		}
	}
}

void AKrakenGameMode::CreateAndShuffleCards()
{
	const int32 PlayerCount = GetEffectivePlayerCount();
	int32 EmptyCount = 0;
	int32 TreasureCount = 0;
	int32 KrakenCardCount = 0;
	GetCardCountsForPlayerCount(PlayerCount, EmptyCount, TreasureCount, KrakenCardCount);

	CardDeck.Empty();
	for (int32 i = 0; i < EmptyCount; i++) CardDeck.Add(ECardType::Empty);
	for (int32 i = 0; i < TreasureCount; i++) CardDeck.Add(ECardType::Treasure);
	for (int32 i = 0; i < KrakenCardCount; i++) CardDeck.Add(ECardType::Kraken);

	for (int32 i = CardDeck.Num() - 1; i > 0; i--)
	{
		CardDeck.Swap(i, FMath::RandRange(0, i));
	}

	UE_LOG(LogTemp, Log, TEXT("[GameMode] Deck: %d empty, %d treasure, %d kraken (total: %d)"),
		   EmptyCount, TreasureCount, KrakenCardCount, CardDeck.Num());
}

void AKrakenGameMode::DistributeCards()
{
	const int32 PlayerCount = GetEffectivePlayerCount();
	const int32 CardsPerPlayer = 5;

	PlayerCards.Empty();
	PlayerCards.SetNum(PlayerCount);
	PlayerCardRevealed.Empty();
	PlayerCardRevealed.SetNum(PlayerCount);

	int32 DeckIndex = 0;
	for (int32 p = 0; p < PlayerCount; p++)
	{
		PlayerCards[p].Empty();
		PlayerCardRevealed[p].Empty();

		for (int32 c = 0; c < CardsPerPlayer; c++)
		{
			if (DeckIndex < CardDeck.Num())
			{
				PlayerCards[p].Add(CardDeck[DeckIndex]);
				PlayerCardRevealed[p].Add(false);
				DeckIndex++;
			}
		}

		for (int32 i = PlayerCards[p].Num() - 1; i > 0; i--)
		{
			PlayerCards[p].Swap(i, FMath::RandRange(0, i));
		}

		if (PlayerControllers.IsValidIndex(p))
		{
			AKrakenPlayerState* PS = PlayerControllers[p]->GetPlayerState<AKrakenPlayerState>();
			if (PS) PS->SetRemainingBoxCount(CardsPerPlayer);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[GameMode] Cards distributed: %d per player, %d players"),
		   CardsPerPlayer, PlayerCount);
}

void AKrakenGameMode::CollectAndRedistributeCards()
{
	CardDeck.Empty();

	for (int32 p = 0; p < PlayerCards.Num(); p++)
	{
		for (int32 c = 0; c < PlayerCards[p].Num(); c++)
		{
			if (!PlayerCardRevealed[p][c])
			{
				CardDeck.Add(PlayerCards[p][c]);
			}
		}
	}

	for (int32 i = CardDeck.Num() - 1; i > 0; i--)
	{
		CardDeck.Swap(i, FMath::RandRange(0, i));
	}

	const int32 PlayerCount = GetEffectivePlayerCount();
	const int32 CardsPerPlayer = (PlayerCount > 0) ? CardDeck.Num() / PlayerCount : 0;

	PlayerCards.Empty();
	PlayerCards.SetNum(PlayerCount);
	PlayerCardRevealed.Empty();
	PlayerCardRevealed.SetNum(PlayerCount);

	int32 DeckIndex = 0;
	for (int32 p = 0; p < PlayerCount; p++)
	{
		PlayerCards[p].Empty();
		PlayerCardRevealed[p].Empty();

		for (int32 c = 0; c < CardsPerPlayer; c++)
		{
			if (DeckIndex < CardDeck.Num())
			{
				PlayerCards[p].Add(CardDeck[DeckIndex]);
				PlayerCardRevealed[p].Add(false);
				DeckIndex++;
			}
		}

		for (int32 i = PlayerCards[p].Num() - 1; i > 0; i--)
		{
			PlayerCards[p].Swap(i, FMath::RandRange(0, i));
		}

		if (PlayerControllers.IsValidIndex(p))
		{
			AKrakenPlayerState* PS = PlayerControllers[p]->GetPlayerState<AKrakenPlayerState>();
			if (PS) PS->SetRemainingBoxCount(CardsPerPlayer);
		}
	}
}

// ============================================================================
// 턴 관리
// ============================================================================

void AKrakenGameMode::HandleBoxSelectionRequest(APlayerController* RequestingPlayer,
												int32 TargetPlayerIndex, int32 BoxIndex)
{
	if (!IsCurrentActionHolder(RequestingPlayer))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] Rejected: Not the action holder"));
		return;
	}

	const int32 RequesterIndex = GetPlayerIndex(RequestingPlayer);

	// 자기 상자 선택 불가 (단, 디버그 모드에서 혼자 플레이 시에만 허용)
	const bool bSoloDebug = bDebugMode && (PlayerControllers.Num() == 1);
	if (!bSoloDebug && RequesterIndex == TargetPlayerIndex)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] Rejected: Cannot select own card"));
		return;
	}

	if (!PlayerCards.IsValidIndex(TargetPlayerIndex) ||
		!PlayerCards[TargetPlayerIndex].IsValidIndex(BoxIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] Rejected: Invalid indices (Player=%d, Box=%d)"),
			   TargetPlayerIndex, BoxIndex);
		return;
	}

	if (PlayerCardRevealed[TargetPlayerIndex][BoxIndex])
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] Rejected: Already revealed"));
		return;
	}

	PendingTargetPlayerIndex = TargetPlayerIndex;
	PendingBoxIndex = BoxIndex;

	AKrakenGameState* KrakenGS = GetGameState<AKrakenGameState>();
	if (KrakenGS)
	{
		KrakenGS->SetPendingSelection(TargetPlayerIndex, BoxIndex);
	}

	UE_LOG(LogTemp, Log, TEXT("[GameMode] Box selected: Player %d, Box %d (by Player %d)"),
		   TargetPlayerIndex, BoxIndex, RequesterIndex);

	// 솔로 디버그: 즉시 확정
	if (bSoloDebug)
	{
		GetWorldTimerManager().ClearTimer(DiscussionTimerHandle);
		TransitionToPhase(EKrakenGamePhase::Reveal);
	}
}

void AKrakenGameMode::HandleConfirmReveal(APlayerController* RequestingPlayer)
{
	if (!IsCurrentActionHolder(RequestingPlayer)) return;

	if (PendingTargetPlayerIndex < 0 || PendingBoxIndex < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] No box selected to confirm"));
		return;
	}

	GetWorldTimerManager().ClearTimer(DiscussionTimerHandle);
	TransitionToPhase(EKrakenGamePhase::Reveal);
}

bool AKrakenGameMode::IsCurrentActionHolder(APlayerController* Player) const
{
	// 솔로 디버그: 항상 허용
	if (bDebugMode && PlayerControllers.Num() == 1)
	{
		return true;
	}

	const int32 PlayerIndex = GetPlayerIndex(Player);
	return PlayerIndex == CurrentActionHolderIndex;
}

EWinCondition AKrakenGameMode::CheckWinCondition() const
{
	if (RevealedTreasureCount >= TotalTreasureCount)
	{
		return EWinCondition::CrewFoundAllTreasure;
	}

	AKrakenGameState* KrakenGS = GetGameState<AKrakenGameState>();
	if (KrakenGS)
	{
		const TArray<FRevealedCardInfo>& Cards = KrakenGS->GetRevealedCards();
		if (Cards.Num() > 0 && Cards.Last().CardType == ECardType::Kraken)
		{
			return EWinCondition::KrakenRevealed;
		}
	}

	return EWinCondition::None;
}

void AKrakenGameMode::OnDiscussionTimeExpired()
{
	UE_LOG(LogTemp, Log, TEXT("[GameMode] Discussion time expired!"));

	if (PendingTargetPlayerIndex >= 0 && PendingBoxIndex >= 0)
	{
		TransitionToPhase(EKrakenGamePhase::Reveal);
	}
	else
	{
		TransitionToPhase(EKrakenGamePhase::Discussion);
	}
}

// ============================================================================
// 헬퍼
// ============================================================================

void AKrakenGameMode::GetRoleCountsForPlayerCount(int32 PlayerCount,
	int32& OutCrewCount, int32& OutKrakenCount) const
{
	switch (PlayerCount)
	{
	case 2:  OutCrewCount = 1; OutKrakenCount = 1; break; // 테스트용
	case 3:  OutCrewCount = 2; OutKrakenCount = 1; break; // 테스트용
	case 4:  OutCrewCount = 3; OutKrakenCount = 1; break;
	case 5:  OutCrewCount = 3; OutKrakenCount = 2; break;
	case 6:  OutCrewCount = 4; OutKrakenCount = 2; break;
	case 7:  OutCrewCount = 5; OutKrakenCount = 2; break;
	case 8:  OutCrewCount = 5; OutKrakenCount = 3; break;
	default: OutCrewCount = 3; OutKrakenCount = 1; break;
	}
}

void AKrakenGameMode::GetCardCountsForPlayerCount(int32 PlayerCount,
	int32& OutEmptyCount, int32& OutTreasureCount, int32& OutKrakenCount) const
{
	const int32 TotalCards = PlayerCount * 5;
	OutTreasureCount = PlayerCount;
	OutKrakenCount = 1;
	OutEmptyCount = TotalCards - OutTreasureCount - OutKrakenCount;
}

APlayerController* AKrakenGameMode::GetPlayerControllerByIndex(int32 Index) const
{
	return PlayerControllers.IsValidIndex(Index) ? PlayerControllers[Index] : nullptr;
}

int32 AKrakenGameMode::GetRemainingCardCount(int32 PlayerIndex) const
{
	if (!PlayerCards.IsValidIndex(PlayerIndex)) return 0;
	int32 Count = 0;
	for (int32 i = 0; i < PlayerCards[PlayerIndex].Num(); i++)
	{
		if (!PlayerCardRevealed[PlayerIndex][i]) Count++;
	}
	return Count;
}
