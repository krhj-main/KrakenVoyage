// KrakenCharacter.h
// ============================================================================
// 1인칭 캐릭터 - 해적선 위를 돌아다니는 플레이어 캐릭터
// ============================================================================
//
// ItemSystem 프로젝트의 AItemSystemCharacter를 기반으로 설계
// 동일한 패턴:
//   - SpringArm + Camera (1인칭)
//   - Enhanced Input System (IA_Move, IA_Look, IA_Interact)
//   - Sphere Trace 기반 상호작용
//
// 변경/추가 사항:
//   - 상호작용 대상이 Door → ExplorationBox로 변경
//   - Sphere Trace 결과를 UI에 표시 (상호작용 힌트)
//   - 멀티플레이어 환경을 고려한 구조
//
// Day7Project에서 배운 점 적용:
//   - Event Tick 대신 타이머 기반 Sphere Trace (0.1초 간격)
//   - SpringArm TargetArmLength = 0 (완전 1인칭)
//   - bUsePawnControlRotation = true (카메라가 컨트롤러 회전 따라감)
//
// ============================================================================

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "KrakenCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class AExplorationBox;
class UVOIPTalker;

// 상호작용 대상이 변경되었을 때 UI 업데이트용 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractTargetChanged, AExplorationBox*, NewTarget);


UCLASS()
class KRAKENVOYAGE_API AKrakenCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AKrakenCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ========================================================================
	// 컴포넌트
	// ========================================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voice")
	UVOIPTalker* VOIPTalker;

	// ========================================================================
	// Enhanced Input 에셋 (에디터에서 할당)
	// ========================================================================
	// 이 프로퍼티들은 블루프린트 서브클래스(BP_KrakenCharacter)에서
	// 에디터의 디테일 패널을 통해 실제 Input Action 에셋을 할당함
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* InteractAction;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "Input")
	UInputAction* ConfirmAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* PushToTalkAction;

	// ========================================================================
	// 상호작용 시스템
	// ========================================================================

	// Sphere Trace로 감지하는 상호작용 범위
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractionRange = 300.0f;

	// Sphere Trace 반경
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractionSphereRadius = 30.0f;

	// Sphere Trace 폴링 간격 (Day7에서 배운 최적화)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractionCheckInterval = 0.1f;

	// 현재 상호작용 가능한 대상 (가장 가까운 ExplorationBox)
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	AExplorationBox* CurrentInteractTarget = nullptr;

	// 상호작용 대상 변경 이벤트 (UI 힌트 표시/숨기기)
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractTargetChanged OnInteractTargetChanged;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* PauseAction;

	void TogglePause();

	// ★ 채팅 토글 (Enter키)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* ChatAction;

	void ToggleChatInput();

protected:

	// ========================================================================
	// 입력 함수
	// ========================================================================

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartInteract();
	void StartConfirm();

	// PTT Start Stop
	void StartPushToTalk();
	void StopPushToTalk();

	// ========================================================================
	// 상호작용 감지 (타이머 기반)
	// ========================================================================

	FTimerHandle InteractionCheckTimerHandle;

	// 타이머에 의해 주기적으로 호출
	void CheckForInteractable();

	// Sphere Trace 수행
	AExplorationBox* PerformInteractionTrace() const;
};
