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
	//virtual void Tick(float DeltaTime) override;

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


    // TODO: 입력 처리 함수 2개
	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);
	void StartSprint();
	void StopSprint();

	UFUNCTION(Server, Reliable)
	void ServerSetSprinting(bool bNewSprinting);
	// ↑ 클라가 호출 → 서버에서 실행. Day 4의 통로②와 동일한 구조

	

};
