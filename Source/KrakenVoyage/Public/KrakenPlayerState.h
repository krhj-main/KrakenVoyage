// KrakenPlayerState.h
// ============================================================================
// 플레이어 스테이트 - 개별 플레이어의 복제 상태
// ============================================================================
//
// PlayerState는 각 플레이어마다 하나씩 생성되며, 네트워크로 복제됨
//
// 보안 핵심: Replication Condition 활용
//   - "남은 상자 수" → 모든 클라이언트에 복제 (DOREPLIFETIME)
//     이유: 다른 플레이어의 남은 상자 수는 공개 정보
//   
//   - "역할(선원/크라켄)" → 게임 중에는 소유자에게만 복제 (COND_OwnerOnly)
//     이유: 다른 플레이어의 역할은 비밀 정보
//     게임 종료 시에만 전체 공개 (bRoleRevealed = true)
//
// ============================================================================

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "KrakenTypes.h"
#include "KrakenPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoleRevealed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBoxCountChanged, int32, NewCount);


UCLASS()
class KRAKENVOYAGE_API AKrakenPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AKrakenPlayerState();

	// ========================================================================
	// 복제되는 프로퍼티
	// ========================================================================

	// 플레이어 역할 (선원/크라켄)
	// 게임 중에는 본인에게만 복제, 게임 종료 시 전체 공개
	UPROPERTY(ReplicatedUsing = OnRep_PlayerRole, BlueprintReadOnly, Category = "Player Info")
	EPlayerRole PlayerRole = EPlayerRole::None;

	// 역할이 전체 공개되었는지 (게임 종료 시 true)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player Info")
	bool bRoleRevealed = false;

	// 남은 상자 수 (모든 클라이언트에 복제 - 공개 정보)
	UPROPERTY(ReplicatedUsing = OnRep_RemainingBoxCount, BlueprintReadOnly, Category = "Player Info")
	int32 RemainingBoxCount = 0;

	// 준비 상태 (로비에서 사용)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player Info")
	bool bIsReady = false;

	// 현재 액션 마커를 가지고 있는지
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player Info")
	bool bHasActionMarker = false;

	// 플레이어 인덱스 (접속 순서, 0부터)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player Info")
	int32 PlayerIndex = -1;

	UPROPERTY(Replicated,BlueprintReadOnly, Category = "Voice")
	bool bIsTalking = false;

	// ========================================================================
	// 서버에서 호출하는 Setter
	// ========================================================================

	// 역할 설정 (GameMode에서 호출)
	void SetRole(EPlayerRole NewRole);

	// 게임 종료 시 역할 공개
	void RevealRole();

	// 남은 상자 수 업데이트
	void SetRemainingBoxCount(int32 Count);

	// 준비 상태 토글
	void SetReady(bool bReady);

	// 액션 마커 상태 설정
	void SetHasActionMarker(bool bHas);

	// ========================================================================
	// 이벤트 델리게이트
	// ========================================================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRoleRevealed OnRoleRevealed;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBoxCountChanged OnBoxCountChanged;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_PlayerRole();

	UFUNCTION()
	void OnRep_RemainingBoxCount();
};
