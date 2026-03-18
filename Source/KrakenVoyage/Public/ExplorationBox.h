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
class UNiagaraComponent;
class UNiagaraSystem;

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

	// USoundBase: UE5에서 모든 사운드 에셋의 부모 클래스
	// SoundCue, SoundWave 모두 여기에 할당 가능
	// EditDefaultsOnly: 블루프린트 Details에서만 설정 가능 (인스턴스별 X)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	USoundBase* Sound_BoxOpen;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	USoundBase* Sound_Empty;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	USoundBase* Sound_Treasure;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	USoundBase* Sound_Kraken;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	USoundBase* Sound_BoxClose;

	// ================================================================
	// ★ 연출 개선: 이펙트
	// ================================================================

	// UNiagaraSystem: UE5의 파티클 이펙트 에셋
	// 보물 발견 시 반짝이 파티클
	UPROPERTY(EditDefaultsOnly, AdvancedDisplay, Category = "FX")
	UNiagaraSystem* FX_TreasureReveal;

	UPROPERTY(EditDefaultsOnly, AdvancedDisplay, Category = "FX")
	UNiagaraSystem* FX_KrakenReveal;

	// ================================================================
	// ★ 연출 개선: 부드러운 애니메이션
	// ================================================================

	// 뚜껑이 열리는 목표 각도 (기본 -110도 = 뒤로 젖혀짐)
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float LidOpenAngle = -110.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float LidOpenSpeed = 200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float LidCloseSpeed = 300.0f;


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

	float CurrentLidAngle = 0.0f;
	float TargetLidAngle = 0.0f;
	bool bIsAnimating = false;
	bool bContentRevealed = false;
	virtual void Tick(float DeltaTime) override;
	void PlayOpenAnimationSmooth();
	void PlayCloseAnimationSmooth();

	// 사운드 재생 헬퍼
	void PlayRevealSound(ECardType InCardType);

	// 이펙트 생성 헬퍼
	void SpawnRevealEffect(ECardType InCardType);
};
