// KrakenTypes.h
// 게임 전체에서 사용하는 열거형(Enum)과 구조체(Struct) 정의
// 이 파일을 별도로 분리하는 이유:
//   - 여러 클래스에서 공통으로 참조하는 타입을 한 곳에서 관리
//   - 순환 참조(Circular Dependency) 방지
//   - 네트워크 Replication에 필요한 UENUM/USTRUCT 매크로를 깔끔하게 정리

#pragma once

#include "CoreMinimal.h"
#include "KrakenTypes.generated.h"

// ============================================================================
// 게임 페이즈 (Game Phase)
// ============================================================================
// 게임의 전체 흐름을 상태 머신(State Machine)으로 관리
// GameState에서 현재 페이즈를 Replicate하면,
// 모든 클라이언트가 동일한 페이즈에 맞는 UI/로직을 실행할 수 있음
//
// 흐름도:
//   WaitingForPlayers → RoleReveal → CardCheck → Discussion 
//   → Reveal → (반복 or RoundTransition) → GameOver
// ============================================================================
UENUM(BlueprintType)
enum class EKrakenGamePhase : uint8
{
	// 로비에서 플레이어를 기다리는 상태
	WaitingForPlayers	UMETA(DisplayName = "Waiting For Players"),

	// 역할 카드를 비밀리에 공개하는 단계 (각자 자기 역할만 확인)
	RoleReveal			UMETA(DisplayName = "Role Reveal"),

	// 카드 확인 단계 - 자신에게 배분된 카드 종류 확인 (위치는 모름)
	CardCheck			UMETA(DisplayName = "Card Check"),

	// 토론 단계 - 보이스/텍스트 채팅으로 토론, 액션 마커 보유자가 상자 선택
	Discussion			UMETA(DisplayName = "Discussion"),

	// 공개 단계 - 선택된 상자를 열어 내용물 공개
	Reveal				UMETA(DisplayName = "Reveal"),

	// 라운드 전환 - 남은 카드 회수, 재셔플, 재분배
	RoundTransition		UMETA(DisplayName = "Round Transition"),

	// 게임 종료 - 승리팀 발표
	GameOver			UMETA(DisplayName = "Game Over")
};


// ============================================================================
// 플레이어 역할 (Player Role)
// ============================================================================
// 핵심 보안 사항:
//   - 서버(GameMode)만 모든 플레이어의 역할을 알고 있음
//   - 각 클라이언트에게는 자기 자신의 역할만 Client RPC로 전송
//   - 다른 플레이어의 역할은 게임 종료 시에만 공개
// ============================================================================
UENUM(BlueprintType)
enum class EPlayerRole : uint8
{
	None		UMETA(DisplayName = "None"),		// 아직 배정되지 않음
	Crew		UMETA(DisplayName = "Crew"),		// 선원 (탐험대)
	Kraken		UMETA(DisplayName = "Kraken")		// 크라켄 일당 (스켈레톤)
};


// ============================================================================
// 탐험 카드 타입 (Exploration Card Type)
// ============================================================================
// 보드게임의 "탐험 카드"에 대응
// 게임에서는 물리적 상자(ExplorationBox)의 내용물로 표현됨
// ============================================================================
UENUM(BlueprintType)
enum class ECardType : uint8
{
	Empty		UMETA(DisplayName = "Empty"),		// 빈 상자
	Treasure	UMETA(DisplayName = "Treasure"),	// 보물 상자
	Kraken		UMETA(DisplayName = "Kraken"),		// 크라켄 (공개 시 크라켄팀 즉시 승리)

	// === 확장 규칙용 (Phase 6에서 구현) ===
	Curse		UMETA(DisplayName = "Curse"),		// 탐욕스러운 선장의 저주
	Special		UMETA(DisplayName = "Special")		// 특수 카드
};


// ============================================================================
// 승리 조건 결과 (Win Condition)
// ============================================================================
UENUM(BlueprintType)
enum class EWinCondition : uint8
{
	None				UMETA(DisplayName = "None"),
	CrewFoundAllTreasure	UMETA(DisplayName = "Crew Found All Treasure"),	// 보물 전부 발견 → 선원 승리
	KrakenRevealed		UMETA(DisplayName = "Kraken Revealed"),				// 크라켄 공개 → 크라켄팀 승리
	TimeRanOut			UMETA(DisplayName = "Time Ran Out")					// 4라운드 종료 → 크라켄팀 승리
};


// ============================================================================
// 카드 공개 기록 (Revealed Card Info)
// ============================================================================
// 게임 중 공개된 카드의 히스토리를 기록하는 구조체
// 결과 화면에서 "누가 누구의 카드를 열었는지" 리플레이에 사용
// ============================================================================
USTRUCT(BlueprintType)
struct FRevealedCardInfo
{
	GENERATED_BODY()

	// 카드를 공개한 플레이어 (액션 마커 보유자)
	UPROPERTY(BlueprintReadOnly)
	int32 RevealerPlayerIndex = -1;

	// 카드를 공개당한 플레이어 (상자 주인)
	UPROPERTY(BlueprintReadOnly)
	int32 TargetPlayerIndex = -1;

	// 공개된 카드의 타입
	UPROPERTY(BlueprintReadOnly)
	ECardType CardType = ECardType::Empty;

	// 몇 라운드에서 공개되었는지
	UPROPERTY(BlueprintReadOnly)
	int32 Round = 0;

	// 해당 라운드 내 몇 번째 턴인지
	UPROPERTY(BlueprintReadOnly)
	int32 TurnInRound = 0;
};


// ============================================================================
// 방 설정 (Room Settings)
// ============================================================================
// 방장이 로비에서 설정하는 게임 옵션
// GameMode에서 이 설정값을 읽어 게임 규칙에 적용
// ============================================================================
USTRUCT(BlueprintType)
struct FRoomSettings
{
	GENERATED_BODY()

	// 최대 플레이어 수 (4~8)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "4", ClampMax = "8"))
	int32 MaxPlayers = 6;

	// 토론 시간 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "30", ClampMax = "180"))
	int32 DiscussionTimeSeconds = 60;

	// 총 라운드 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "3", ClampMax = "5"))
	int32 MaxRounds = 4;

	// 확장 규칙: 탐욕스러운 선장의 저주
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseCurseExpansion = false;

	// 확장 규칙: 특수 카드
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseSpecialCards = false;
};
