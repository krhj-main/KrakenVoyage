#include "KVGameMode.h"
#include "KVGameState.h"
#include "KVPlayerController.h"

AKVGameMode::AKVGameMode()
{
    GameStateClass = AKVGameState::StaticClass();
    PlayerControllerClass = AKVPlayerController::StaticClass();
    // ↑ "이 게임모드는 이 GameState와 PlayerController를 쓴다"고 지정.
    //   생성자에서 하는 건 클래스 지정일 뿐 로직이 아니므로 안전하다 (Day 3 ①단계 규칙).
}

void AKVGameMode::HandleOpenBoxRequest(AKVPlayerController* Requester, int32 BoxIndex)
{
    if (!Requester) { return; }

    // --- 검증 자리 (Day 6에서 채운다) ---
    // 지금 이 플레이어에게 행동권이 있는가?
    // 그 상자가 남의 구역인가?
    // 이미 열린 상자는 아닌가?
    // → 전부 여기 한 곳에 모을 것. 개요 4.2의 지시.

    // 임시: 짝수 인덱스면 촉수라고 치자
    const bool bIsTentacle = (BoxIndex % 2 == 0);

    if (AKVGameState* GS = GetGameState<AKVGameState>())
        // ↑ GameMode는 서버에만 있으므로, 여기서 GameState 접근은 항상 안전
    {
        if (bIsTentacle)
        {
            GS->AddKrakenGauge(1);              // 통로①: Replicated 변수 (상태)
            GS->MulticastKrakenRoar();   // 통로③: Multicast (일회성 연출)
        }
    }

    // 통로④: Client RPC — 요청한 사람에게만 비밀 통보
    Requester->ClientReceiveZoneComposition(2, 2, 1);
}