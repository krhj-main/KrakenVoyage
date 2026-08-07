// Fill out your copyright notice in the Description page of Project Settings.


#include "KVExplorationBox.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "KVGameState.h"

// Sets default values
AKVExplorationBox::AKVExplorationBox()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    bReplicates = true;
    // ↑ 이 액터를 클라이언트에도 복제하겠다는 선언. 없으면 서버에만 존재한다.

    BoxBase = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxBase"));
    RootComponent = BoxBase;
    // 해석: 해당 소스를 상속받는 개체는 기본적으로 위 객체를 가지고있음. 위 객체는 루트 컴포넌트로 설정이라는 뜻

    LidPivot = CreateDefaultSubobject<USceneComponent>(TEXT("LidPivot"));
    LidPivot->SetupAttachment(BoxBase);
    LidPivot->SetRelativeLocation(FVector(-50.f, 0.f, 50.f));
    // ↑ 상자 뒤쪽 위 모서리. 실제 메시 크기에 맞춰 조정할 것
    // 해석 : 이것도 기본적으로 가지고있을 객체, 그리고 객체에 관한 세부설정 BoxBase에 부착되고 기본위치설정

    BoxLid = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxLid"));
    BoxLid->SetupAttachment(LidPivot);
    BoxLid->SetRelativeLocation(FVector(50.f, 0.f, 0.f));
    // ↑ 피벗에서 앞쪽으로. 피벗이 회전하면 이 뚜껑이 호를 그리며 열린다

    ContentMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ContentMesh"));
    ContentMesh->SetupAttachment(BoxBase);
    ContentMesh->SetRelativeLocation(FVector(0.f, 0.f, 20.f));
    ContentMesh->SetVisibility(false);
    ContentMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    // ↑ 내용물은 처음엔 안 보이고, 충돌도 없어야 LineTrace를 방해하지 않는다
    // 해석 : 위의 설정들과 같고 보이지않는 설정과 LineTrace에 걸리지않도록 설정

}

// Called when the game starts or when spawned
void AKVExplorationBox::BeginPlay()
{
	Super::BeginPlay();
    ContentMesh->SetVisibility(false);
	// 해석 : 안전을 위해 게임 시작되고서도 내용물은 안 보이도록 설정
}

// 이해하지못한 함수부분이다
void AKVExplorationBox::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AKVExplorationBox, bIsOpen);
    // ↑ Content는 여기 없다. 의도적이다.
}

void AKVExplorationBox::OpenBox()
{
    if (!HasAuthority()) { return; }
    if (bIsOpen) { return; }          // 이미 열린 상자는 무시
    // 해석 : 서버권한을 갖고있을때만 열수있음, 열려있으면 무시

    bIsOpen = true;

    MulticastPlayOpenEffect(Content);
    // ↑ 이 호출로 내용물이 처음 클라이언트에게 전달된다.
    //   그 전까지 클라는 이 상자에 뭐가 들었는지 알 수 없다.

    if (Content == EKVBoxContent::Tentacle)
    {
        if (AKVGameState* GS = GetWorld()->GetGameState<AKVGameState>())
        {
            GS->AddKrakenGauge(1);
            GS->MulticastKrakenRoar();
        }
    }
    // 해석 : 열린 상자가 텐타클이면 게임스테이트를 가져와 크라켄 게이지를 올리고, 크라켄 효과를 재생시킨다.
}

// OnRep은 어떨때 구현하는가? 어느 함수에 구현해야하는가?
void AKVExplorationBox::OnRep_IsOpen()
{
    // 중간 접속자를 위한 처리 자리.
    // 지금은 Multicast가 연출을 담당하므로 비워둔다.
    // (Day 7에서 "이미 열린 상자를 열린 상태로 그리기"를 여기에 넣는다)
}

// Implementation은 무엇인가? 왜 붙이는가? 꼭해야되는것은? 잘모르겠다.
void AKVExplorationBox::MulticastPlayOpenEffect_Implementation(EKVBoxContent RevealedContent)
{
    bAnimating = true;
    AnimProgress = 0.f;

    ApplyContentVisual(RevealedContent);

    const FString Where = HasAuthority() ? TEXT("[SERVER]") : TEXT("[CLIENT]");
    const TCHAR* Name =
        RevealedContent == EKVBoxContent::Treasure ? TEXT("보물") :
        RevealedContent == EKVBoxContent::Tentacle ? TEXT("촉수") : TEXT("빈 상자");

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Yellow,
            FString::Printf(TEXT("%s 상자가 열렸다: %s"), *Where, Name));
    }
}

void AKVExplorationBox::ApplyContentVisual(EKVBoxContent InContent)
{
    ContentMesh->SetVisibility(InContent != EKVBoxContent::Empty);
    // ↑ 빈 상자면 아무것도 안 보인다

    // 색은 Day 5에서는 생략 가능. 개요 7장 기준:
    // 빈=회색, 보물=금색, 촉수=보라
    // 머티리얼 인스턴스를 BP에서 지정하는 방식이 편하다 (변형 과제 참조)
}

void AKVExplorationBox::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bAnimating) { return; }

    AnimProgress += DeltaTime / OpenDuration;
    AnimProgress = FMath::Clamp(AnimProgress, 0.f, 1.f);

    const float Eased = FMath::InterpEaseOut(0.f, 1.f, AnimProgress, 2.f);
    // ↑ 끝에서 부드럽게 감속. 뚜껑이 툭 멈추지 않고 스르륵 멎는다

    const float Angle = FMath::Lerp(0.f, -LidOpenAngle, Eased);
    LidPivot->SetRelativeRotation(FRotator(Angle, 0.f, 0.f));
    // ↑ FRotator(Pitch, Yaw, Roll) — 뚜껑은 Pitch로 젖혀진다

    if (AnimProgress >= 1.f)
    {
        bAnimating = false;
    }
    // 해석 : 전체적으로 박스가 열리는 애니메이션에 관한 함수구현
}

