// KrakenGameState.h
// ============================================================================
// 게임 스테이트 - 모든 클라이언트에 복제되는 게임 전체 상태
// ============================================================================
//
// GameState vs GameMode 차이:
//   - GameMode: 서버에만 존재, 게임 규칙의 "심판"
//   - GameState: 서버→모든 클라이언트 자동 복제, "전광판" 역할
//
// 여기에 저장하는 정보 = "모든 플레이어가 알아야 하는 공개 정보"
//   - 현재 라운드, 턴
//   - 공개된 카드 목록
//   - 현재 게임 페이즈
//   - 누가 액션 마커를 가지고 있는지
//
// 여기에 저장하면 안 되는 정보:
//   - 비공개 카드의 내용 (서버 GameMode에만)
//   - 다른 플레이어의 역할 (게임 종료 전까지)
//
// ============================================================================

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "KrakenTypes.h"
#include "KrakenGameState.generated.h"

// ============================================================================
// OnRep 함수가 호출될 때 클라이언트에서 UI를 업데이트하기 위한 델리게이트
// ============================================================================
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhaseChanged, EKrakenGamePhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRevealedCardsUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTurnInfoUpdated);


UCLASS()
class KRAKENVOYAGE_API AKrakenGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AKrakenGameState();

	// ========================================================================
	// 복제되는 게임 상태 (Replicated Properties)
	// ========================================================================
	// UPROPERTY(ReplicatedUsing=함수명) 패턴:
	//   서버에서 값이 변경되면 → 자동으로 클라이언트에 복제 → OnRep 함수 호출
	//   OnRep 함수에서 UI 업데이트 등 클라이언트 로직 실행

	// 현재 게임 페이즈
	UPROPERTY(ReplicatedUsing = OnRep_CurrentPhase, BlueprintReadOnly, Category = "Game State")
	EKrakenGamePhase CurrentPhase = EKrakenGamePhase::WaitingForPlayers;

	// 현재 라운드 (1부터 시작)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	int32 CurrentRound = 0;

	// 현재 라운드 내 턴 번호
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	int32 CurrentTurnInRound = 0;

	// 최대 라운드 수
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	int32 MaxRounds = 4;

	// 액션 마커를 보유한 플레이어 인덱스
	UPROPERTY(ReplicatedUsing = OnRep_TurnInfo, BlueprintReadOnly, Category = "Game State")
	int32 ActionHolderPlayerIndex = -1;

	// 공개된 카드 목록 (게임 히스토리)
	UPROPERTY(ReplicatedUsing = OnRep_RevealedCards, BlueprintReadOnly, Category = "Game State")
	TArray<FRevealedCardInfo> RevealedCards;

	// 현재 공개된 보물 수
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	int32 RevealedTreasureCount = 0;

	// 필요한 총 보물 수
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	int32 TotalTreasureCount = 0;

	// 승리 조건 (게임 종료 시)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	EWinCondition WinResult = EWinCondition::None;

	// 현재 선택 중인 상자 (마커 위치 표시용)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	int32 PendingSelectionPlayerIndex = -1;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	int32 PendingSelectionBoxIndex = -1;

	// 토론 타이머 남은 시간 (초)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	float DiscussionTimeRemaining = 0.0f;

	// ========================================================================
	// 서버에서 호출하는 Setter 함수들
	// ========================================================================

	void SetCurrentPhase(EKrakenGamePhase NewPhase);
	void SetTurnInfo(int32 Round, int32 Turn, int32 ActionHolder);
	void AddRevealedCard(const FRevealedCardInfo& CardInfo);
	void SetWinCondition(EWinCondition Condition);
	void SetPendingSelection(int32 PlayerIndex, int32 BoxIndex);

	// ========================================================================
	// Getter
	// ========================================================================

	const TArray<FRevealedCardInfo>& GetRevealedCards() const { return RevealedCards; }

	// ========================================================================
	// 이벤트 델리게이트 (UI 바인딩용)
	// ========================================================================
	// 블루프린트에서 이 이벤트에 바인딩하면
	// 값이 바뀔 때마다 UI 위젯이 자동 업데이트됨

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPhaseChanged OnPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRevealedCardsUpdated OnRevealedCardsUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTurnInfoUpdated OnTurnInfoUpdated;

protected:

	// ========================================================================
	// Replication 설정
	// ========================================================================

	// UE5 네트워크에서 어떤 프로퍼티를 복제할지 등록하는 필수 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ========================================================================
	// OnRep 콜백 (클라이언트에서 값이 복제되었을 때 호출)
	// ========================================================================

	UFUNCTION()
	void OnRep_CurrentPhase();

	UFUNCTION()
	void OnRep_RevealedCards();

	UFUNCTION()
	void OnRep_TurnInfo();
};
