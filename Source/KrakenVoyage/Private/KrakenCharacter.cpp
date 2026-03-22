// KrakenCharacter.cpp

#include "KrakenCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ExplorationBox.h"
#include "Net/VoiceConfig.h"
#include "KrakenPlayerController.h"
#include "KrakenGameState.h"



AKrakenCharacter::AKrakenCharacter()
{
	// Tick은 OFF - 상호작용 감지는 타이머로 처리 (Day7 최적화 경험)
	PrimaryActorTick.bCanEverTick = false;

	// ====================================================================
	// SpringArm 생성 및 설정 (ItemSystem과 동일한 패턴)
	// ====================================================================
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetCapsuleComponent());
	
	// 캡슐 상단 눈높이로 위치 조정
	// 기본 캡슐 절반높이 88, 눈높이는 약 60~70
	SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 60.0f));
	
	// TargetArmLength = 0 → 완전 1인칭 (카메라가 SpringArm 위치에 바로 붙음)
	SpringArm->TargetArmLength = 0.0f;
	
	// 카메라가 컨트롤러의 회전을 따라감 (마우스 움직임 → 시점 회전)
	SpringArm->bUsePawnControlRotation = true;
	
	// 벽 충돌 테스트 OFF (1인칭이라 필요 없음)
	SpringArm->bDoCollisionTest = false;

	// ====================================================================
	// Camera 생성 및 설정
	// ====================================================================
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false;

	//VOIPTalker
	VOIPTalker = CreateDefaultSubobject<UVOIPTalker>(TEXT("VOIPTalker"));
	

	// ====================================================================
	// CharacterMovement 설정
	// ====================================================================
	GetCharacterMovement()->MaxWalkSpeed = 300.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 1500.0f;
	GetCharacterMovement()->JumpZVelocity = 400.0f;

	// ====================================================================
	// 컨트롤러 회전 설정
	// ====================================================================
	// Yaw만 캐릭터 몸체에 적용 (좌우 회전 시 몸체도 같이 회전)
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
}

void AKrakenCharacter::BeginPlay()
{
	Super::BeginPlay();

	// ====================================================================
	// Enhanced Input Mapping Context 등록
	// ====================================================================
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	// ====================================================================
	// 상호작용 감지 타이머 시작 (0.1초 간격)
	// ====================================================================
	// Event Tick에서 매 프레임 Sphere Trace하는 대신
	// 타이머로 간격을 두고 체크 → 성능 최적화
	GetWorldTimerManager().SetTimer(InteractionCheckTimerHandle,
		this, &AKrakenCharacter::CheckForInteractable,
		InteractionCheckInterval, true);
}

void AKrakenCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// Tick은 사용하지 않음 - 모든 주기적 체크는 타이머로 처리
}

// ============================================================================
// 입력 바인딩
// ============================================================================

void AKrakenCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput =
		Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// WASD 이동
		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, 
									  this, &AKrakenCharacter::Move);
		}

		// 마우스 시점
		if (LookAction)
		{
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, 
									  this, &AKrakenCharacter::Look);
		}

		// 점프 (스페이스바)
		if (JumpAction)
		{
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Triggered, 
									  this, &ACharacter::Jump);
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, 
									  this, &ACharacter::StopJumping);
		}

		// 상호작용 (E키)
		if (InteractAction)
		{
			EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, 
									  this, &AKrakenCharacter::StartInteract);
		}

		if (ConfirmAction)
		{
			EnhancedInput->BindAction(ConfirmAction, ETriggerEvent::Started,
				this, &AKrakenCharacter::StartConfirm);
		}

		if (PushToTalkAction)
		{
			EnhancedInput->BindAction(PushToTalkAction, ETriggerEvent::Started,
				this, &AKrakenCharacter::StartPushToTalk);
			EnhancedInput->BindAction(PushToTalkAction, ETriggerEvent::Completed,
				this, &AKrakenCharacter::StopPushToTalk);
		}

		if (PauseAction)
		{
			EnhancedInput->BindAction(PauseAction, ETriggerEvent::Started,
				this, &AKrakenCharacter::TogglePause);
		}

		if (ChatAction)
		{
			EnhancedInput->BindAction(ChatAction, ETriggerEvent::Started,
				this, &AKrakenCharacter::ToggleChatInput);
		}
	}
}

// ============================================================================
// 이동 (ItemSystem과 동일한 로직)
// ============================================================================

void AKrakenCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	// 컨트롤러의 Yaw 회전만 사용 → 캐릭터가 바라보는 방향 기준 이동
	const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

	const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDir, MovementVector.Y);  // W/S
	AddMovementInput(RightDir, MovementVector.X);     // A/D
}

// ============================================================================
// 시점 (ItemSystem과 동일한 로직)
// ============================================================================

void AKrakenCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookVector.X);    // 좌우
	AddControllerPitchInput(LookVector.Y);  // 상하
}

// ============================================================================
// 상호작용 (E키 입력)
// ============================================================================

void AKrakenCharacter::StartInteract()
{
	if (!CurrentInteractTarget)
	{
		UE_LOG(LogTemp, Log, TEXT("[Character] No interact target"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[Character] Interacting with box: %s"), 
		   *CurrentInteractTarget->GetName());

	// PlayerController를 통해 서버에 상자 선택 요청
	AKrakenPlayerController* KPC = Cast<AKrakenPlayerController>(Controller);
	if (KPC)
	{
		// ExplorationBox에서 소유자 인덱스와 상자 인덱스 가져오기
		const int32 TargetPlayerIdx = CurrentInteractTarget->OwnerPlayerIndex;
		const int32 BoxIdx = CurrentInteractTarget->BoxIndex;

		// Server RPC 호출 → 서버의 GameMode에서 처리
		KPC->ServerSelectBox(TargetPlayerIdx, BoxIdx);
	}
}

// ============================================================================
// 상호작용 감지 (타이머 기반 Sphere Trace)
// ============================================================================

void AKrakenCharacter::CheckForInteractable()
{
	AExplorationBox* NewTarget = PerformInteractionTrace();

	// 대상이 변경되었을 때만 업데이트
	if (NewTarget != CurrentInteractTarget)
	{
		CurrentInteractTarget = NewTarget;
		OnInteractTargetChanged.Broadcast(NewTarget);
	}
}

AExplorationBox* AKrakenCharacter::PerformInteractionTrace() const
{
	if (!Camera)
	{
		return nullptr;
	}

	// 카메라 위치에서 카메라가 바라보는 방향으로 Sphere Trace
	const FVector TraceStart = Camera->GetComponentLocation();
	const FVector TraceEnd = TraceStart + Camera->GetForwardVector() * InteractionRange;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);  // 자기 자신은 무시

	const bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		FQuat::Identity,
		ECC_Visibility,  // Visibility 채널 사용
		FCollisionShape::MakeSphere(InteractionSphereRadius),
		QueryParams
	);

	if (bHit)
	{
		// 히트된 액터가 ExplorationBox인지 확인
		return Cast<AExplorationBox>(HitResult.GetActor());
	}

	return nullptr;
}

void AKrakenCharacter::StartConfirm()
{
	AKrakenPlayerController* KPC = Cast<AKrakenPlayerController>(Controller);
	if (KPC)
	{
		KPC->ServerConfirmReveal();
		UE_LOG(LogTemp, Log, TEXT("[Character] Confirm pressed -> ServerConfirmReveal"));
	}
}

void AKrakenCharacter::StartPushToTalk()
{
	// ★ 토론/게임오버 페이즈에서만 말할 수 있음
	// 다른 페이즈에서는 PTT를 눌러도 무시
	AKrakenGameState* GS = GetWorld()->GetGameState<AKrakenGameState>();
	if (GS)
	{
		const bool bCanTalk = (GS->CurrentPhase == EKrakenGamePhase::Discussion) || (GS->CurrentPhase == EKrakenGamePhase::GameOver);

		if (!bCanTalk)
		{
			UE_LOG(LogTemp, Verbose, TEXT("[Voice] Cannot talk in this phase"));
			return;
		}
	}
	// GetOwner의 PlayerController를 통해 콘솔 명령 실행
	// "ToggleSpeaking 1" = UE5 내장 명령으로 마이크를 켬
	// 이 명령은 VOIPTalker가 있는 Pawn에서 자동으로 음성 캡처 시작
	APlayerController* PC = Cast<APlayerController>(Controller);
	if (PC)
	{
		PC->ConsoleCommand(TEXT("ToggleSpeaking 1"));
		UE_LOG(LogTemp, Log, TEXT("[Voice] PTT ON"));

		// 서버에 "나 지금 말하는 중" 상태 전달 (UI 표시용)
		AKrakenPlayerController* KPC = Cast<AKrakenPlayerController>(PC);
		if (KPC)
		{
			KPC->ServerSetTalking(true);
		}
	}
}

void AKrakenCharacter::StopPushToTalk()
{
	APlayerController* PC = Cast<APlayerController>(Controller);
	if (PC)
	{
		PC->ConsoleCommand(TEXT("ToggleSpeaking 0"));
		UE_LOG(LogTemp, Log, TEXT("[Voice] PTT OFF"));

		AKrakenPlayerController* KPC = Cast<AKrakenPlayerController>(PC);
		if (KPC)
		{
			KPC->ServerSetTalking(false);
		}
	}
}
void AKrakenCharacter::TogglePause()
{
	AKrakenPlayerController* KPC = Cast<AKrakenPlayerController>(Controller);
	if (KPC) KPC->TogglePauseMenu();
}

void AKrakenCharacter::ToggleChatInput()
{
	AKrakenPlayerController* KPC = Cast<AKrakenPlayerController>(Controller);
	if (KPC)
	{
		KPC->ToggleChat();
	}
}