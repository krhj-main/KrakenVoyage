// KrakenGameInstance.h
// ============================================================================
// 게임 인스턴스 - 레벨 전환 시에도 유지되는 전역 상태
// ============================================================================
//
// GameInstance의 역할:
//   - 게임 실행 중 단 하나만 존재 (싱글톤처럼)
//   - 레벨이 바뀌어도 파괴되지 않음
//   - 메인 메뉴 → 로비 → 인게임 전환 관리
//   - 세션 생성/참가/검색 (나중에 Steam 연동)
//
// 흐름:
//   메인 메뉴 (L_MainMenu)
//     → 방 만들기 → 로비 레벨로 이동 (Listen Server)
//     → 방 참가 → IP/코드로 접속
//     → 인게임 레벨로 이동
//
// ============================================================================

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "KrakenGameInstance.generated.h"


UCLASS()
class KRAKENVOYAGE_API UKrakenGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UKrakenGameInstance();

	// ========================================================================
	// 레벨 전환
	// ========================================================================

	// 메인 메뉴로 이동
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void GoToMainMenu();

	// 호스트: 로비 레벨을 Listen Server로 열기
	UFUNCTION(BlueprintCallable, Category = "Session")
	void HostGame(const FString& PlayerName);

	// 클라이언트: IP 주소로 서버에 접속
	UFUNCTION(BlueprintCallable, Category = "Session")
	void JoinGame(const FString& IPAddress, const FString& PlayerName);

	// ========================================================================
	// 플레이어 정보
	// ========================================================================

	UPROPERTY(BlueprintReadWrite, Category = "Player Info")
	FString LocalPlayerName = TEXT("Player");

	// ========================================================================
	// 맵 경로
	// ========================================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maps")
	FString MainMenuMapPath = TEXT("/Game/Maps/L_MainMenu");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maps")
	FString GameMapPath = TEXT("/Game/Maps/L_Ship_Prototype");
};
