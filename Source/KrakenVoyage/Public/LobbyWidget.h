// LobbyWidget.h
// ============================================================================
// 로비 위젯 - 방에 입장한 후 게임 시작 전 대기 화면
// ============================================================================
//
// 레이아웃:
//   ┌──────────────────────────────────┐
//   │  LOBBY                           │
//   │                                  │
//   │  Players:                        │
//   │    Player1 (Host)    [Ready ✓]   │
//   │    Player2           [Ready ✗]   │
//   │                                  │
//   │  [ Ready / Not Ready ]           │
//   │  [ Start Game ] (호스트만)        │
//   │  [ Leave ]                       │
//   │                                  │
//   │  Room Code: 127.0.0.1:17777      │
//   └──────────────────────────────────┘
//
// 현재 Phase 3에서는 심플한 형태로 구현
// 인게임 맵에서 게임 시작 전 Discussion 이전에 표시
//
// ============================================================================

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyWidget.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;

UCLASS()
class KRAKENVOYAGE_API ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// ========================================================================
	// BindWidget
	// ========================================================================

	// 플레이어 목록 표시 영역
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_PlayerList;

	// 현재 접속 인원
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_PlayerCount;

	// 준비 상태 버튼
	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Ready;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_ReadyLabel;

	// 게임 시작 버튼 (호스트만 보임)
	UPROPERTY(meta = (BindWidget))
	UButton* Btn_StartGame;

	// 나가기 버튼
	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Leave;

	// 상태 메시지
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Status;

protected:
	UFUNCTION()
	void OnReadyClicked();

	UFUNCTION()
	void OnStartGameClicked();

	UFUNCTION()
	void OnLeaveClicked();

	void UpdateLobbyDisplay();

	bool bIsReady = false;
};
