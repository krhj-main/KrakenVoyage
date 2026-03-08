// ExplorationBox.h
// ============================================================================
// 탐험 상자 - 보드게임의 "탐험 카드"를 3D 공간의 상자로 구현
// ============================================================================
//
// 에디터에서 배치할 때:
//   1. BP_ExplorationBox를 선실에 배치
//   2. 디테일 패널에서 OwnerPlayerIndex (0~7)와 BoxIndex (0~4) 설정
//   3. GameMode가 StartGame 시 이 값을 읽어서 카드와 연결
//
// ============================================================================

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KrakenTypes.h"
#include "ExplorationBox.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBoxOpened, ECardType, RevealedType);


UCLASS()
class KRAKENVOYAGE_API AExplorationBox : public AActor
{
	GENERATED_BODY()

public:
	AExplorationBox();

	// ========================================================================
	// 컴포넌트
	// ========================================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* BoxMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* LidPivot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* LidMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* InteractionCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* ContentMesh;

	// ========================================================================
	// 상자 정보
	// ========================================================================
	
	// 에디터에서 배치할 때 설정하는 값
	// EditAnywhere: 에디터 디테일 패널에서 수정 가능
	// Replicated: 서버→클라이언트 자동 복제
	
	// 이 상자를 소유한 플레이어 인덱스 (0부터 시작)
	// 선실 A의 상자들 = 0, 선실 B = 1, 선실 C = 2, 선실 D = 3
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Box Info",
		meta = (ClampMin = "0", ClampMax = "7"))
	int32 OwnerPlayerIndex = 0;

	// 이 상자의 인덱스 (해당 플레이어의 몇 번째 상자인지, 0~4)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Box Info",
		meta = (ClampMin = "0", ClampMax = "4"))
	int32 BoxIndex = 0;

	// 공개 여부
	UPROPERTY(ReplicatedUsing = OnRep_bIsRevealed, BlueprintReadOnly, Category = "Box Info")
	bool bIsRevealed = false;

	// 공개된 카드 타입 (공개 후에만 유효)
	UPROPERTY(ReplicatedUsing = OnRep_RevealedCardType, BlueprintReadOnly, Category = "Box Info")
	ECardType RevealedCardType = ECardType::Empty;

	// ========================================================================
	// 서버에서 호출하는 함수
	// ========================================================================

	void InitializeBox(int32 InOwnerPlayerIndex, int32 InBoxIndex);
	void RevealBox(ECardType CardType);
	void ResetBox();

	// ========================================================================
	// 이벤트
	// ========================================================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBoxOpened OnBoxOpened;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_bIsRevealed();

	UFUNCTION()
	void OnRep_RevealedCardType();

	void PlayOpenAnimation();
	void UpdateContentVisual(ECardType CardType);
};
