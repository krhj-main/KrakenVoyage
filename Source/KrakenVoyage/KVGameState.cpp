#include "KVGameState.h"
#include "Net/UnrealNetwork.h"   // DOREPLIFETIME 매크로가 여기 있다 (3번 함정)

void AKVGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    // ↑ Super 호출 필수. 부모가 복제하던 것들이 사라진다.

    //DOREPLIFETIME(AKVGameState, KrakenGauge);
    // ↑ "이 변수를 전원에게 복제하라" 등록
}

void AKVGameState::AddKrakenGauge(int32 Amount)
{
    if (!HasAuthority()) { return; }
    // ↑ 컨벤션: 상태 변경 함수 첫 줄에 권위 가드.
    //   클라이언트가 이 값을 바꿔봐야 다음 복제 때 서버 값으로 덮어써진다.

    KrakenGauge += Amount;

    // ⚠️ 중요: OnRep은 서버에서 실행되지 않는다.
    //   리슨 서버 호스트 화면도 갱신되려면 직접 불러줘야 한다.
    OnRep_KrakenGauge();
}

void AKVGameState::OnRep_KrakenGauge()
{
    const FString Where = HasAuthority() ? TEXT("[SERVER]") : TEXT("[CLIENT]");
    // ↑ 이 줄이 오늘의 학습 도구. 로그가 어느 머신에서 찍혔는지 눈으로 확인한다.

    const FString Msg = FString::Printf(TEXT("%s 크라켄 게이지: %d / 3"), *Where, KrakenGauge);

    UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, Msg);
        // ↑ -1 = 새 줄로 추가, 5.f = 5초간 표시
    }
}

void AKVGameState::MulticastKrakenRoar_Implementation()
{
    const FString Where = HasAuthority() ? TEXT("[SERVER]") : TEXT("[CLIENT]");
    const FString Msg = FString::Printf(TEXT("%s *** 배 전체에 굉음이 울린다 ***"), *Where);
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);
    if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Msg); }
}