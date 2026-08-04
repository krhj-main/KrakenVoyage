#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "KVGameMode.generated.h"

class AKVPlayerController;

UCLASS()
class KRAKENVOYAGE_API AKVGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AKVGameMode();

    // 개요 4.2의 "판정 함수". 서버에서만 호출된다.
    // 지금은 검증 내용이 비어있지만, 자리를 먼저 잡아둔다.
    void HandleOpenBoxRequest(AKVPlayerController* Requester, int32 BoxIndex);
};