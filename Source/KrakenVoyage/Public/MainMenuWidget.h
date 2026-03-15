// MainMenuWidget.h
// ============================================================================
// 메인 메뉴 위젯 - 게임 타이틀 화면
// ============================================================================
//
// 레이아웃:
//   ┌──────────────────────────────┐
//   │                              │
//   │      KRAKEN'S VOYAGE         │
//   │                              │
//   │     [ 이름 입력 ]            │
//   │     [ 방 만들기 ]            │
//   │     [ 방 참가   ]            │
//   │     [ 설정      ]            │
//   │     [ 종료      ]            │
//   │                              │
//   └──────────────────────────────┘
//
// ============================================================================

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UEditableTextBox;
class UTextBlock;

UCLASS()
class KRAKENVOYAGE_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// ========================================================================
	// BindWidget - 블루프린트 위젯과 자동 연결
	// ========================================================================

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Title;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* Input_PlayerName;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Host;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Join;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Settings;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Quit;

	// 방 참가 시 IP 입력 패널 (토글로 보이기/숨기기)
	UPROPERTY(meta = (BindWidgetOptional))
	UEditableTextBox* Input_IPAddress;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_ConnectIP;

protected:
	// ========================================================================
	// 버튼 클릭 핸들러
	// ========================================================================

	UFUNCTION()
	void OnHostClicked();

	UFUNCTION()
	void OnJoinClicked();

	UFUNCTION()
	void OnSettingsClicked();

	UFUNCTION()
	void OnQuitClicked();

	UFUNCTION()
	void OnConnectIPClicked();
};
