// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "KVCharacter.generated.h"

class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class KRAKENVOYAGE_API AKVCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AKVCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;

    // TODO: 카메라 컴포넌트 1개
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

    // TODO: Input 자산 슬롯 4개
    //       (MappingContext 1 + InputAction 3: Move, Look, Jump)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SprintAction;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float WalkSpeed = 600.f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float SprintSpeed = 1500.f;

	// 박스부분
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditAnywhere, Category = "Interaction")
	float InteractDistance = 300.f;

	UPROPERTY()
	TObjectPtr<class AKVExplorationBox> FocusedBox;   // 지금 바라보는 상자


    // TODO: 입력 처리 함수 2개
	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);
	void StartSprint();
	void StopSprint();

	// 박스 부분 함수
	void Interact();
	void TraceForBox();

	UFUNCTION(Server, Reliable)
	void ServerSetSprinting(bool bNewSprinting);
	// ↑ 클라가 호출 → 서버에서 실행. Day 4의 통로②와 동일한 구조


	UFUNCTION(Server, Reliable)
	void ServerRequestOpenBox(AKVExplorationBox* Box);
	// ↑ 설계 판단의 답: 캐릭터에 둔다.
	//   캐릭터는 소유 클라이언트가 소유하므로 Server RPC가 서버로 건너간다.
	//   상자는 아무도 소유하지 않아 상자에 두면 RPC가 무시된다.
	//   대상 액터는 인자로 넘긴다 — 액터 포인터는 복제 대상이면 자동 변환된다.
	

};
