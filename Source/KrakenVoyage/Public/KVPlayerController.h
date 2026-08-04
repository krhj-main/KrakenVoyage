#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "KVPlayerController.generated.h"

UCLASS()
class KRAKENVOYAGE_API AKVPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    // 콘솔 명령. PIE에서 ` 키를 눌러 "KVOpenBox 0" 입력하면 호출된다.
    UFUNCTION(Exec)
    void KVOpenBox(int32 BoxIndex);
    // ↑ Exec: 콘솔에서 직접 부를 수 있게 하는 지정자.
    //   입력 시스템 없이 기능을 테스트할 때 매우 유용하다.

    // 통로②: 클라 → 서버 요청
    UFUNCTION(Server, Reliable)
    void ServerRequestOpenBox(int32 BoxIndex);
    // ↑ Server: 이 함수는 "소유 클라이언트에서 호출하면 서버에서 실행"된다.
    //   Reliable: 반드시 도착 보장 (게임 로직에 영향을 주므로)
    //   본문은 ServerRequestOpenBox_Implementation 으로 작성한다. (컨벤션)


    // 통로④: 서버 → 이 컨트롤러의 주인에게만
    UFUNCTION(Client, Reliable)
    void ClientReceiveZoneComposition(int32 Treasure, int32 Empty, int32 Tentacle);
};