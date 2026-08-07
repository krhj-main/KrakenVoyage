// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KVExplorationBox.generated.h"


UENUM(BlueprintType)
enum class EKVBoxContent : uint8
{
	Empty     UMETA(DisplayName = "빈 상자"),
	Treasure  UMETA(DisplayName = "보물"),
	Tentacle  UMETA(DisplayName = "촉수")
};
// ↑ uint8 기반이어야 BP에서 쓸 수 있다. UMETA는 에디터 표시 이름.
// UE환경에서 enum을 어떻게 사용하고 선언하는지 몰라서 헤맸음. 내가아는 지식은 enumd에 열거형 데이터를 담을수있다 정도

class UStaticMeshComponent;

UCLASS()
class KRAKENVOYAGE_API AKVExplorationBox : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AKVExplorationBox();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void OpenBox();

	bool IsOpen() const { return bIsOpen; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>&) const override;
	// 이것은 무엇인가?

	// --- 컴포넌트 ---
	// 이 부분은 미리 구현한것과 같은 형식이됐음 성공.
	UPROPERTY(VisibleAnywhere, Category = "Box")
	TObjectPtr<UStaticMeshComponent> BoxBase;

	UPROPERTY(VisibleAnywhere, Category = "Box")
	TObjectPtr<USceneComponent> LidPivot;      // 경첩 역할

	UPROPERTY(VisibleAnywhere, Category = "Box")
	TObjectPtr<UStaticMeshComponent> BoxLid;

	UPROPERTY(VisibleAnywhere, Category = "Box")
	TObjectPtr<UStaticMeshComponent> ContentMesh;

	// --- 상태 ---
	// ⚠️ Content는 복제하지 않는다. 서버만 아는 진실.
	// 전혀 감을 못잡은 구간
	UPROPERTY(EditAnywhere, Category = "Box")
	EKVBoxContent Content = EKVBoxContent::Empty;
	// ↑ EditAnywhere라 레벨에 배치한 뒤 각 상자의 내용물을 손으로 지정할 수 있다.
	//   Day 6에서 GameMode가 자동 배분하게 바꾼다.

	// 이부분도 잘 이해가 가지않음 ReplicatedUsing 형식과 = OnRep_IsOpen이 따라오는 형식이 무엇일까?
	UPROPERTY(ReplicatedUsing = OnRep_IsOpen)
	bool bIsOpen = false;

	// OnRep 형식은 어느때 써야하고 어떤 역할인가?
	UFUNCTION()
	void OnRep_IsOpen();

	// 이 아래부분으로는 쭉 잘모르겠음 !
	// 전원 연출 — 이 순간 내용물이 공개된다
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayOpenEffect(EKVBoxContent RevealedContent);
	// ↑ Reliable인 이유: 이건 단순 연출이 아니라 정보 공개다. 유실되면 안 된다.

	// --- 애니메이션 ---
	UPROPERTY(EditAnywhere, Category = "Box")
	float LidOpenAngle = -110.f;    // 개요 7장

	UPROPERTY(EditAnywhere, Category = "Box")
	float OpenDuration = 1.0f;      // 초

	bool bAnimating = false;
	float AnimProgress = 0.f;

	void ApplyContentVisual(EKVBoxContent InContent);
};
