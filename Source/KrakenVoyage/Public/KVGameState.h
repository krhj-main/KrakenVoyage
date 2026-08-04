#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "KVGameState.generated.h"

UCLASS()
class KRAKENVOYAGE_API AKVGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    // 크라켄 게이지 — 전원 공개 상태. Day 3 표의 "GameState = 공개 정보" 적용
    UPROPERTY(BlueprintReadOnly, Category = "Kraken")
    int32 KrakenGauge = 0;
    // ↑ ReplicatedUsing: 값이 클라에 도착하면 지정한 함수를 호출하라는 뜻.
    //   그냥 Replicated로 쓰면 값만 조용히 바뀌고 알림이 없다.

    

    // 서버 전용: 게이지를 올린다
    void AddKrakenGauge(int32 Amount);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    // ↑ "이 클래스에서 복제할 변수 목록"을 UE에 신고하는 함수.
    //   UPROPERTY에 Replicated를 써도 여기 등록 안 하면 복제 안 된다. (1번 함정)

    UFUNCTION(NetMulticast, Unreliable)
    void MulticastKrakenRoar();
    // ↑ GameState는 전원에게 복제되므로 Multicast가 진짜로 전원에게 도달한다

protected:
    UFUNCTION()
    void OnRep_KrakenGauge();
    // ↑ UFUNCTION() 필수. 빼면 OnRep이 호출되지 않는다. (2번 함정)
};