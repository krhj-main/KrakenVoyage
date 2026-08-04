#include "KVPlayerController.h"
#include "KVGameMode.h"
#include "Engine/Engine.h"

// 로그 헬퍼 — 어느 머신에서 찍혔는지 표시
static void KVLog(const UObject* Ctx, bool bAuthority, const FString& Text)
{
    const FString Where = bAuthority ? TEXT("[SERVER]") : TEXT("[CLIENT]");
    const FString Msg = FString::Printf(TEXT("%s %s"), *Where, *Text);
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, Msg);
    }
}

void AKVPlayerController::KVOpenBox(int32 BoxIndex)
{
    KVLog(this, HasAuthority(), FString::Printf(TEXT("콘솔 입력: 상자 %d"), BoxIndex));

    ServerRequestOpenBox(BoxIndex);
    // ↑ 여기가 핵심. 클라이언트에서 호출 → 서버로 건너간다.
    //   서버(호스트)에서 호출하면? 그냥 로컬 함수처럼 즉시 실행된다. (Q14의 답)
}

void AKVPlayerController::ServerRequestOpenBox_Implementation(int32 BoxIndex)
{
    // 이 함수 본문은 항상 서버에서 실행된다. 여기 로그는 [SERVER]만 찍힌다.
    KVLog(this, HasAuthority(), FString::Printf(TEXT("요청 수신: 상자 %d"), BoxIndex));

    if (AKVGameMode* GM = GetWorld()->GetAuthGameMode<AKVGameMode>())
        // ↑ 클라이언트였다면 여기서 nullptr. 하지만 이 함수는 서버에서만 도니 안전하다.
    {
        GM->HandleOpenBoxRequest(this, BoxIndex);
    }
}


void AKVPlayerController::ClientReceiveZoneComposition_Implementation(
    int32 Treasure, int32 Empty, int32 Tentacle)
{
    // 이 컨트롤러를 소유한 클라이언트에서만 실행된다.
    KVLog(this, HasAuthority(),
        FString::Printf(TEXT("[비밀] 내 구역: 보물%d 빈%d 촉수%d"), Treasure, Empty, Tentacle));
}