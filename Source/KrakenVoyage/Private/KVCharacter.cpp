// Fill out your copyright notice in the Description page of Project Settings.


#include "KVCharacter.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/CapsuleComponent.h"
#include "InputActionValue.h"
#include "KVExplorationBox.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AKVCharacter::AKVCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 60.0f));
	FirstPersonCamera->bUsePawnControlRotation = true;

	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	
	

}

// Called when the game starts or when spawned
void AKVCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
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
	
}


// Called every frame
void AKVCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsLocallyControlled())
	{
		TraceForBox();
		// ↑ 내 화면의 문제이므로 소유 클라에서만. 서버가 남의 시선을 매 프레임 계산할 이유가 없다
	}

}



void AKVCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();

	if (Controller)
	{
		//Pitch Roll 을 버리고 Yaw만 사용
		const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
		const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);	

		AddMovementInput(Forward, Axis.Y);
		AddMovementInput(Right, Axis.X);
	}
}

void AKVCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();

	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void AKVCharacter::StartSprint()
{
	ServerSetSprinting(true);
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}
void AKVCharacter::StopSprint()
{
	ServerSetSprinting(false);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}
void AKVCharacter::ServerSetSprinting_Implementation(bool bNewSprinting)
{
	GetCharacterMovement()->MaxWalkSpeed = bNewSprinting ? SprintSpeed : WalkSpeed;
}

// 박스 부분
void AKVCharacter::TraceForBox()
{
	const FVector Start = FirstPersonCamera->GetComponentLocation();
	const FVector End = Start + FirstPersonCamera->GetComponentRotation().Vector() * InteractDistance;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FHitResult Hit;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_Visibility, Params);

	FocusedBox = bHit ? Cast<AKVExplorationBox>(Hit.GetActor()) : nullptr;
	// ↑ Cast는 타입이 안 맞으면 nullptr을 준다. 별도 검사가 필요 없다

	if (FocusedBox && !FocusedBox->IsOpen() && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 0.1f, FColor::Green, TEXT("[E] 상자 열기"));
		// ↑ 키를 1로 고정 → 매 프레임 같은 줄을 덮어쓴다 (-1이면 화면이 도배된다)
	}
}

void AKVCharacter::Interact()
{
	if (FocusedBox)
	{
		ServerRequestOpenBox(FocusedBox);
	}
}

void AKVCharacter::ServerRequestOpenBox_Implementation(AKVExplorationBox* Box)
{
	if (!Box) { return; }

	// --- 검증 자리 (Day 6) ---
	// 거리가 실제로 가까운가? (클라가 조작한 요청 차단)
	// 이 플레이어에게 행동권이 있는가?
	// 남의 구역 상자인가?

	Box->OpenBox();
}

// Called to bind functionality to input
void AKVCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Enhanced Input
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AKVCharacter::Move);
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AKVCharacter::Look);
		// ↑ Triggered = 입력이 활성인 매 틱. 이동/시점에 적합

		EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);	
		// ↑ Started = 누른 순간, Completed = 뗀 순간

		EIC->BindAction(SprintAction, ETriggerEvent::Started, this, &AKVCharacter::StartSprint);
		EIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &AKVCharacter::StopSprint);
		// 달리기 바인드액션

		//박스부분 바인딩
		EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &AKVCharacter::Interact);
	}

}

