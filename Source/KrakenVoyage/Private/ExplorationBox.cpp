// ExplorationBox.cpp

#include "ExplorationBox.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"

AExplorationBox::AExplorationBox()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// ====================================================================
	// Replication 활성화
	// ====================================================================
	// 이 Actor가 네트워크에서 복제되도록 설정
	// 서버에서 스폰하면 자동으로 클라이언트에도 생성됨
	bReplicates = true;

	// ====================================================================
	// 상자 본체 메쉬
	// ====================================================================
	BoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxMesh"));
	SetRootComponent(BoxMesh);
	// 프로토타입: 기본 큐브 메쉬는 블루프린트에서 할당
	// 또는 ConstructorHelpers로 엔진 기본 큐브 로드 가능 (추후)

	// ====================================================================
	// 뚜껑 피벗 (Day7의 HingePoint 패턴)
	// ====================================================================
	// 뚜껑이 상자 윗면의 한쪽 끝을 기준으로 열리도록
	// Scene Component를 피벗으로 사용
	LidPivot = CreateDefaultSubobject<USceneComponent>(TEXT("LidPivot"));
	LidPivot->SetupAttachment(BoxMesh);
	// 피벗 위치는 상자 윗면 뒤쪽 가장자리 (블루프린트에서 조정)
	LidPivot->SetRelativeLocation(FVector(-25.0f, 0.0f, 25.0f));

	LidMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LidMesh"));
	LidMesh->SetupAttachment(LidPivot);
	// 뚜껑은 피벗으로부터의 상대 위치로 설정 (블루프린트에서 조정)

	// ====================================================================
	// 내용물 메쉬 (상자 내부에 위치, 열릴 때 보임)
	// ====================================================================
	ContentMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ContentMesh"));
	ContentMesh->SetupAttachment(BoxMesh);
	ContentMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 10.0f));
	ContentMesh->SetVisibility(false);  // 초기에는 숨김

	// ====================================================================
	// 상호작용 콜리전 (Sphere Trace 감지용)
	// ====================================================================
	InteractionCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionCollision"));
	InteractionCollision->SetupAttachment(BoxMesh);
	InteractionCollision->SetBoxExtent(FVector(40.0f, 40.0f, 40.0f));
	InteractionCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void AExplorationBox::BeginPlay()
{
	Super::BeginPlay();
}

// ============================================================================
// Replication 설정
// ============================================================================

void AExplorationBox::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AExplorationBox, OwnerPlayerIndex);
	DOREPLIFETIME(AExplorationBox, BoxIndex);
	DOREPLIFETIME(AExplorationBox, bIsRevealed);
	DOREPLIFETIME(AExplorationBox, RevealedCardType);
}

// ============================================================================
// 서버 함수
// ============================================================================

void AExplorationBox::InitializeBox(int32 InOwnerPlayerIndex, int32 InBoxIndex)
{
	if (HasAuthority())
	{
		OwnerPlayerIndex = InOwnerPlayerIndex;
		BoxIndex = InBoxIndex;
		bIsRevealed = false;
		RevealedCardType = ECardType::Empty;

		// 비주얼 리셋
		ContentMesh->SetVisibility(false);

		UE_LOG(LogTemp, Log, TEXT("[ExplorationBox] Initialized: Owner=%d, Index=%d"),
			   OwnerPlayerIndex, BoxIndex);
	}
}

void AExplorationBox::RevealBox(ECardType CardType)
{
	if (HasAuthority())
	{
		bIsRevealed = true;
		RevealedCardType = CardType;

		// 서버에서도 비주얼 업데이트 (Listen Server인 경우 서버도 화면이 있으므로)
		OnRep_bIsRevealed();
		OnRep_RevealedCardType();
	}
}
// ============================================================================
// OnRep 콜백 (클라이언트 비주얼 업데이트)
// ============================================================================

void AExplorationBox::OnRep_bIsRevealed()
{
	if (bIsRevealed)
	{
		PlayOpenAnimation();
		UpdateContentVisual(RevealedCardType);
	}
}

void AExplorationBox::OnRep_RevealedCardType()
{
	if (bIsRevealed)
	{
		UpdateContentVisual(RevealedCardType);
		OnBoxOpened.Broadcast(RevealedCardType);
	}
}

// ============================================================================
// 비주얼
// ============================================================================

void AExplorationBox::PlayOpenAnimation()
{
	PlayOpenAnimationSmooth();

	UE_LOG(LogTemp, Log, TEXT("[ExplorationBox] Box opened! Owner=%d, Index=%d, Type=%d"),
		   OwnerPlayerIndex, BoxIndex, static_cast<int32>(RevealedCardType));

	// TODO Phase 6: Timeline을 사용한 부드러운 열기 애니메이션
	// TODO Phase 6: 크라켄 카드 공개 시 특수 이펙트 (파티클, 사운드)
}

void AExplorationBox::UpdateContentVisual(ECardType CardType)
{
	ContentMesh->SetVisibility(true);

	// ====================================================================
	// 프로토타입: 색상으로 내용물 구분
	// ====================================================================
	// 나중에 실제 모델링으로 교체:
	//   보물 → 금화가 든 상자
	//   빈 상자 → 텅 빈 내부
	//   크라켄 → 촉수가 튀어나오는 연출

	UMaterialInstanceDynamic* DynMat = ContentMesh->CreateDynamicMaterialInstance(0);
	if (DynMat)
	{
		switch (CardType)
		{
		case ECardType::Treasure:
			DynMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(1.0f, 0.84f, 0.0f));  // 금색
			break;
		case ECardType::Kraken:
			DynMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.5f, 0.0f, 0.5f));  // 보라색
			break;
		case ECardType::Empty:
		default:
			DynMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.4f, 0.3f, 0.2f));  // 갈색
			break;
		}
	}
}

void AExplorationBox::PlayOpenAnimationSmooth()
{
	TargetLidAngle = LidOpenAngle;
	bIsAnimating = true;
	bContentRevealed = false;

	// Tick 활성화 (애니메이션 시작)
	// SetActorTickEnabled: 해당 액터의 Tick을 켜거나 끄는 함수
	// 성능을 위해 필요할 때만 Tick 활성화
	SetActorTickEnabled(true);

	// 뚜껑 열리는 소리 재생
	if (Sound_BoxOpen)
	{
		// PlaySoundAtLocation: 3D 공간의 특정 위치에서 소리 재생
		// 가까이 있으면 크게, 멀리 있으면 작게 들림
		UGameplayStatics::PlaySoundAtLocation(
			this, Sound_BoxOpen, GetActorLocation(), 1.0f, 1.0f);
	}
	UE_LOG(LogTemp, Log, TEXT("[Box] Opening smooth: %s"), *GetName());
}

void AExplorationBox::PlayCloseAnimationSmooth()
{
	TargetLidAngle = 0.0f;
	bIsAnimating = true;
	bContentRevealed = false;
	SetActorTickEnabled(true);

	if (Sound_BoxClose)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Sound_BoxClose, GetActorLocation());
	}
}

void AExplorationBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsAnimating || !LidPivot) return;

	const float Speed = (TargetLidAngle < CurrentLidAngle) ? LidOpenSpeed : LidCloseSpeed;

	CurrentLidAngle = FMath::FInterpTo(CurrentLidAngle, TargetLidAngle, DeltaTime, Speed / 60.0f);

	FRotator NewRotation = FRotator(CurrentLidAngle, 0.0f, 0.0f);
	LidPivot->SetRelativeRotation(NewRotation);

	const float RevealThreshold = LidOpenAngle * 0.5f;
	if (!bContentRevealed && CurrentLidAngle <= RevealThreshold)
	{
		bContentRevealed = true;

		PlayRevealSound(RevealedCardType);
		SpawnRevealEffect(RevealedCardType);

		UpdateContentVisual(RevealedCardType);
	}

	if (FMath::IsNearlyEqual(CurrentLidAngle, TargetLidAngle, 0.5f))
	{
		CurrentLidAngle = TargetLidAngle;
		bIsAnimating = false;

		SetActorTickEnabled(false);

		UE_LOG(LogTemp, Log, TEXT("[Box] Animation complete: %s (Angle: %.1f)"),
			*GetName(), CurrentLidAngle);
	}
}

void AExplorationBox::PlayRevealSound(ECardType InCardType)
{
	USoundBase* SoundToPlay = nullptr;

	switch (InCardType)
	{
	case ECardType::Empty:
		SoundToPlay = Sound_Empty;
		break;
	case ECardType::Treasure:
		SoundToPlay = Sound_Treasure;
		break;
	case ECardType::Kraken:
		SoundToPlay = Sound_Kraken;
		break;
	default:
		break;
	}

	if (SoundToPlay)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this, SoundToPlay, GetActorLocation(),
			1.0f,   // 볼륨
			1.0f    // 피치
		);
	}
}

// ============================================================
// ★ 카드 종류별 파티클 이펙트
// ============================================================
void AExplorationBox::SpawnRevealEffect(ECardType InCardType)
{
	UNiagaraSystem* EffectToSpawn = nullptr;

	if (InCardType == ECardType::Treasure && FX_TreasureReveal)
	{
		EffectToSpawn = FX_TreasureReveal;
	}
	else if (InCardType == ECardType::Kraken && FX_KrakenReveal)
	{
		EffectToSpawn = FX_KrakenReveal;
	}

	if (EffectToSpawn)
	{
		// UNiagaraFunctionLibrary::SpawnSystemAtLocation
		// 특정 위치에 Niagara 파티클을 일회성으로 스폰
		// 상자 위치에서 약간 위로 (Z+50) 오프셋
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			EffectToSpawn,
			GetActorLocation() + FVector(0.0f, 0.0f, 50.0f),  // 상자 위
			FRotator::ZeroRotator,
			FVector(1.0f),    // 스케일
			true,             // bAutoDestroy
			true              // bAutoActivate
		);
	}
}

// ============================================================
// ★ 기존 ResetBox 수정 — 부드럽게 닫기
// ============================================================
void AExplorationBox::ResetBox()
{
	bIsRevealed = false;
	RevealedCardType = ECardType::Empty;

	// 즉시 닫기 대신 부드러운 닫기
	PlayCloseAnimationSmooth();

	// ContentMesh 숨기기
	if (ContentMesh)
	{
		ContentMesh->SetVisibility(false);
	}
}