// KrakenPlayerState.cpp

#include "KrakenPlayerState.h"
#include "Net/UnrealNetwork.h"

AKrakenPlayerState::AKrakenPlayerState()
{
}

// ============================================================================
// Replication 설정
// ============================================================================
// 
// 핵심 보안 포인트:
// PlayerRole은 COND_OwnerOnly를 사용하여 본인에게만 복제
// → 다른 플레이어 클라이언트의 메모리에는 이 값이 아예 없음
// → 메모리 해킹으로도 다른 사람의 역할을 볼 수 없음
//
// 단, bRoleRevealed가 true가 되면 (게임 종료 시)
// RevealRole()에서 별도 Multicast로 전체 공개
// ============================================================================
void AKrakenPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 역할은 소유자에게만 복제 (게임 중 비밀 유지)
	DOREPLIFETIME_CONDITION(AKrakenPlayerState, PlayerRole, COND_OwnerOnly);

	// 나머지는 모든 클라이언트에 복제 (공개 정보)
	DOREPLIFETIME(AKrakenPlayerState, bRoleRevealed);
	DOREPLIFETIME(AKrakenPlayerState, RemainingBoxCount);
	DOREPLIFETIME(AKrakenPlayerState, bIsReady);
	DOREPLIFETIME(AKrakenPlayerState, bHasActionMarker);
	DOREPLIFETIME(AKrakenPlayerState, PlayerIndex);
}

// ============================================================================
// Setter 함수들 (서버에서만 호출)
// ============================================================================

void AKrakenPlayerState::SetRole(EPlayerRole NewRole)
{
	if (HasAuthority())
	{
		PlayerRole = NewRole;
	}
}

void AKrakenPlayerState::RevealRole()
{
	if (HasAuthority())
	{
		bRoleRevealed = true;
		// 게임 종료 시에는 역할을 전체 공개해야 하므로
		// COND_OwnerOnly로 복제되는 PlayerRole 대신
		// OnRep에서 델리게이트로 UI에 알림
		// 
		// 주의: PlayerRole 자체는 여전히 COND_OwnerOnly이므로
		// 다른 클라이언트에는 직접 복제되지 않음
		// → 게임 종료 시 Multicast RPC로 전체 역할 정보를 별도 전송해야 함
		// (KrakenGameMode::BeginGameOverPhase에서 처리)
		OnRep_PlayerRole();
	}
}

void AKrakenPlayerState::SetRemainingBoxCount(int32 Count)
{
	if (HasAuthority())
	{
		RemainingBoxCount = Count;
	}
}

void AKrakenPlayerState::SetReady(bool bReady)
{
	if (HasAuthority())
	{
		bIsReady = bReady;
	}
}

void AKrakenPlayerState::SetHasActionMarker(bool bHas)
{
	if (HasAuthority())
	{
		bHasActionMarker = bHas;
	}
}

// ============================================================================
// OnRep 콜백
// ============================================================================

void AKrakenPlayerState::OnRep_PlayerRole()
{
	UE_LOG(LogTemp, Log, TEXT("[PlayerState] Role updated: %d"), static_cast<int32>(PlayerRole));
	OnRoleRevealed.Broadcast();
}

void AKrakenPlayerState::OnRep_RemainingBoxCount()
{
	UE_LOG(LogTemp, Log, TEXT("[PlayerState] Remaining boxes: %d"), RemainingBoxCount);
	OnBoxCountChanged.Broadcast(RemainingBoxCount);
}
