// KrakenGameState.cpp

#include "KrakenGameState.h"
#include "Net/UnrealNetwork.h"  // DOREPLIFETIME 매크로를 위해 필요

AKrakenGameState::AKrakenGameState()
{
}

// ============================================================================
// GetLifetimeReplicatedProps
// ============================================================================
// UE5 네트워크 Replication의 핵심!
// 여기에 등록한 프로퍼티만 서버→클라이언트로 자동 복제됨
//
// DOREPLIFETIME(클래스명, 프로퍼티명)
//   → 값이 변경될 때마다 모든 클라이언트에 복제
//
// DOREPLIFETIME_CONDITION(클래스명, 프로퍼티명, 조건)
//   → 조건에 맞는 클라이언트에만 복제
//   예: COND_OwnerOnly → 소유자(해당 플레이어)에게만 복제
// ============================================================================
void AKrakenGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 모든 클라이언트에 복제 (공개 정보)
	DOREPLIFETIME(AKrakenGameState, CurrentPhase);
	DOREPLIFETIME(AKrakenGameState, CurrentRound);
	DOREPLIFETIME(AKrakenGameState, CurrentTurnInRound);
	DOREPLIFETIME(AKrakenGameState, MaxRounds);
	DOREPLIFETIME(AKrakenGameState, ActionHolderPlayerIndex);
	DOREPLIFETIME(AKrakenGameState, RevealedCards);
	DOREPLIFETIME(AKrakenGameState, RevealedTreasureCount);
	DOREPLIFETIME(AKrakenGameState, TotalTreasureCount);
	DOREPLIFETIME(AKrakenGameState, WinResult);
	DOREPLIFETIME(AKrakenGameState, PendingSelectionPlayerIndex);
	DOREPLIFETIME(AKrakenGameState, PendingSelectionBoxIndex);
	DOREPLIFETIME(AKrakenGameState, DiscussionTimeRemaining);
}

// ============================================================================
// Setter 함수 (서버에서만 호출)
// ============================================================================

void AKrakenGameState::SetCurrentPhase(EKrakenGamePhase NewPhase)
{
	// HasAuthority() = 서버인지 확인
	if (HasAuthority())
	{
		CurrentPhase = NewPhase;
		// 서버에서도 OnRep를 수동 호출 (OnRep은 클라이언트에서만 자동 호출되므로)
		OnRep_CurrentPhase();
	}
}

void AKrakenGameState::SetTurnInfo(int32 Round, int32 Turn, int32 ActionHolder)
{
	if (HasAuthority())
	{
		CurrentRound = Round;
		CurrentTurnInRound = Turn;
		ActionHolderPlayerIndex = ActionHolder;
		OnRep_TurnInfo();
	}
}

void AKrakenGameState::AddRevealedCard(const FRevealedCardInfo& CardInfo)
{
	if (HasAuthority())
	{
		RevealedCards.Add(CardInfo);

		if (CardInfo.CardType == ECardType::Treasure)
		{
			RevealedTreasureCount++;
		}

		OnRep_RevealedCards();
	}
}

void AKrakenGameState::SetWinCondition(EWinCondition Condition)
{
	if (HasAuthority())
	{
		WinResult = Condition;
	}
}

void AKrakenGameState::SetPendingSelection(int32 PlayerIndex, int32 BoxIndex)
{
	if (HasAuthority())
	{
		PendingSelectionPlayerIndex = PlayerIndex;
		PendingSelectionBoxIndex = BoxIndex;
	}
}

// ============================================================================
// OnRep 콜백 (Replication 완료 시 클라이언트에서 호출)
// ============================================================================
// 이 함수들이 호출되면 UI 위젯을 업데이트해야 함
// 델리게이트를 Broadcast하여 바인딩된 UI 위젯에 알림

void AKrakenGameState::OnRep_CurrentPhase()
{
	UE_LOG(LogTemp, Log, TEXT("[GameState] Phase changed to: %d"), static_cast<int32>(CurrentPhase));
	OnPhaseChanged.Broadcast(CurrentPhase);
}

void AKrakenGameState::OnRep_RevealedCards()
{
	UE_LOG(LogTemp, Log, TEXT("[GameState] Revealed cards updated. Total: %d"), RevealedCards.Num());
	OnRevealedCardsUpdated.Broadcast();
}

void AKrakenGameState::OnRep_TurnInfo()
{
	UE_LOG(LogTemp, Log, TEXT("[GameState] Turn info: Round %d, Turn %d, ActionHolder: %d"),
		   CurrentRound, CurrentTurnInRound, ActionHolderPlayerIndex);
	OnTurnInfoUpdated.Broadcast();
}
