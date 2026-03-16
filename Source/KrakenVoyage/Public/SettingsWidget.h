// SettingsWidget.h
// ============================================================================
// 설정 위젯 - 그래픽, 사운드, 조작 설정
// ============================================================================
//
// 레이아웃:
//   ┌──────────────────────────────────────┐
//   │  SETTINGS                            │
//   │                                      │
//   │  [Graphics] [Sound] [Controls]  탭   │
//   │  ─────────────────────────────       │
//   │                                      │
//   │  Graphics:                           │
//   │    Window Mode: [Fullscreen ▼]       │
//   │    Resolution:  [1920x1080 ▼]        │
//   │    Quality:     [High ▼]             │
//   │    VSync:       [✓]                  │
//   │    FPS Limit:   [60 ▼]              │
//   │                                      │
//   │  Sound:                              │
//   │    Master:  ████████░░ 80%           │
//   │    BGM:     ██████░░░░ 60%           │
//   │    SFX:     ████████░░ 80%           │
//   │                                      │
//   │  Controls:                           │
//   │    Mouse Sensitivity: ████░░ 40%     │
//   │    Invert Y: [ ]                     │
//   │                                      │
//   │         [Apply]  [Back]              │
//   └──────────────────────────────────────┘
//
// ============================================================================

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsWidget.generated.h"

class UButton;
class UTextBlock;
class USlider;
class UComboBoxString;
class UCheckBox;
class UWidgetSwitcher;

UCLASS()
class KRAKENVOYAGE_API USettingsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// ========================================================================
	// 탭 전환
	// ========================================================================

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_TabGraphics;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_TabSound;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_TabControls;

	// 탭 내용을 전환하는 Widget Switcher
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* TabSwitcher;

	// ========================================================================
	// Graphics 탭
	// ========================================================================

	UPROPERTY(meta = (BindWidget))
	UComboBoxString* Combo_WindowMode;

	UPROPERTY(meta = (BindWidget))
	UComboBoxString* Combo_Resolution;

	UPROPERTY(meta = (BindWidget))
	UComboBoxString* Combo_Quality;

	UPROPERTY(meta = (BindWidget))
	UCheckBox* Check_VSync;

	UPROPERTY(meta = (BindWidget))
	UComboBoxString* Combo_FPSLimit;

	// ========================================================================
	// Sound 탭
	// ========================================================================

	UPROPERTY(meta = (BindWidget))
	USlider* Slider_Master;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_MasterValue;

	UPROPERTY(meta = (BindWidget))
	USlider* Slider_BGM;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_BGMValue;

	UPROPERTY(meta = (BindWidget))
	USlider* Slider_SFX;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_SFXValue;

	// ========================================================================
	// Controls 탭
	// ========================================================================

	UPROPERTY(meta = (BindWidget))
	USlider* Slider_MouseSensitivity;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_SensitivityValue;

	UPROPERTY(meta = (BindWidget))
	UCheckBox* Check_InvertY;

	// ========================================================================
	// 하단 버튼
	// ========================================================================

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Apply;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Back;

protected:

	// ========================================================================
	// 탭 전환
	// ========================================================================

	UFUNCTION()
	void OnTabGraphicsClicked();

	UFUNCTION()
	void OnTabSoundClicked();

	UFUNCTION()
	void OnTabControlsClicked();

	// ========================================================================
	// 슬라이더 변경
	// ========================================================================

	UFUNCTION()
	void OnMasterVolumeChanged(float Value);

	UFUNCTION()
	void OnBGMVolumeChanged(float Value);

	UFUNCTION()
	void OnSFXVolumeChanged(float Value);

	UFUNCTION()
	void OnMouseSensitivityChanged(float Value);

	// ========================================================================
	// 버튼
	// ========================================================================

	UFUNCTION()
	void OnApplyClicked();

	UFUNCTION()
	void OnBackClicked();

	// ========================================================================
	// 초기화
	// ========================================================================

	void PopulateGraphicsOptions();
	void LoadCurrentSettings();
	void ApplyGraphicsSettings();
	void ApplySoundSettings();
	void SaveSettings();
};
