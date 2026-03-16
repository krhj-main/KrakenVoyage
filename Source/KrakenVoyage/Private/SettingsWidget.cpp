// SettingsWidget.cpp

#include "SettingsWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Slider.h"
#include "Components/ComboBoxString.h"
#include "Components/CheckBox.h"
#include "Components/WidgetSwitcher.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"

void USettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 탭 버튼 바인딩
	if (Btn_TabGraphics)  Btn_TabGraphics->OnClicked.AddDynamic(this, &USettingsWidget::OnTabGraphicsClicked);
	if (Btn_TabSound)     Btn_TabSound->OnClicked.AddDynamic(this, &USettingsWidget::OnTabSoundClicked);
	if (Btn_TabControls)  Btn_TabControls->OnClicked.AddDynamic(this, &USettingsWidget::OnTabControlsClicked);

	// 슬라이더 바인딩
	if (Slider_Master)           Slider_Master->OnValueChanged.AddDynamic(this, &USettingsWidget::OnMasterVolumeChanged);
	if (Slider_BGM)              Slider_BGM->OnValueChanged.AddDynamic(this, &USettingsWidget::OnBGMVolumeChanged);
	if (Slider_SFX)              Slider_SFX->OnValueChanged.AddDynamic(this, &USettingsWidget::OnSFXVolumeChanged);
	if (Slider_MouseSensitivity) Slider_MouseSensitivity->OnValueChanged.AddDynamic(this, &USettingsWidget::OnMouseSensitivityChanged);

	// 하단 버튼 바인딩
	if (Btn_Apply) Btn_Apply->OnClicked.AddDynamic(this, &USettingsWidget::OnApplyClicked);
	if (Btn_Back)  Btn_Back->OnClicked.AddDynamic(this, &USettingsWidget::OnBackClicked);

	// 콤보박스 옵션 채우기
	PopulateGraphicsOptions();

	// 현재 설정 로드
	LoadCurrentSettings();

	// 첫 번째 탭(Graphics)으로 시작
	if (TabSwitcher)
	{
		TabSwitcher->SetActiveWidgetIndex(0);
	}

	UE_LOG(LogTemp, Log, TEXT("[Settings] Widget constructed."));
}

// ============================================================================
// 탭 전환
// ============================================================================

void USettingsWidget::OnTabGraphicsClicked()
{
	if (TabSwitcher) TabSwitcher->SetActiveWidgetIndex(0);
}

void USettingsWidget::OnTabSoundClicked()
{
	if (TabSwitcher) TabSwitcher->SetActiveWidgetIndex(1);
}

void USettingsWidget::OnTabControlsClicked()
{
	if (TabSwitcher) TabSwitcher->SetActiveWidgetIndex(2);
}

// ============================================================================
// 콤보박스 옵션 채우기
// ============================================================================

void USettingsWidget::PopulateGraphicsOptions()
{
	// Window Mode
	if (Combo_WindowMode)
	{
		Combo_WindowMode->ClearOptions();
		Combo_WindowMode->AddOption(TEXT("Fullscreen"));
		Combo_WindowMode->AddOption(TEXT("Windowed Fullscreen"));
		Combo_WindowMode->AddOption(TEXT("Windowed"));
	}

	// Resolution
	if (Combo_Resolution)
	{
		Combo_Resolution->ClearOptions();
		Combo_Resolution->AddOption(TEXT("1920x1080"));
		Combo_Resolution->AddOption(TEXT("2560x1440"));
		Combo_Resolution->AddOption(TEXT("3840x2160"));
		Combo_Resolution->AddOption(TEXT("1600x900"));
		Combo_Resolution->AddOption(TEXT("1280x720"));
	}

	// Quality
	if (Combo_Quality)
	{
		Combo_Quality->ClearOptions();
		Combo_Quality->AddOption(TEXT("Low"));
		Combo_Quality->AddOption(TEXT("Medium"));
		Combo_Quality->AddOption(TEXT("High"));
		Combo_Quality->AddOption(TEXT("Epic"));
	}

	// FPS Limit
	if (Combo_FPSLimit)
	{
		Combo_FPSLimit->ClearOptions();
		Combo_FPSLimit->AddOption(TEXT("30"));
		Combo_FPSLimit->AddOption(TEXT("60"));
		Combo_FPSLimit->AddOption(TEXT("120"));
		Combo_FPSLimit->AddOption(TEXT("144"));
		Combo_FPSLimit->AddOption(TEXT("Unlimited"));
	}
}

// ============================================================================
// 현재 설정 로드
// ============================================================================

void USettingsWidget::LoadCurrentSettings()
{
	UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings();
	if (!UserSettings) return;

	// === Graphics ===

	// Window Mode
	if (Combo_WindowMode)
	{
		const EWindowMode::Type WinMode = UserSettings->GetFullscreenMode();
		switch (WinMode)
		{
		case EWindowMode::Fullscreen:           Combo_WindowMode->SetSelectedOption(TEXT("Fullscreen")); break;
		case EWindowMode::WindowedFullscreen:    Combo_WindowMode->SetSelectedOption(TEXT("Windowed Fullscreen")); break;
		case EWindowMode::Windowed:              Combo_WindowMode->SetSelectedOption(TEXT("Windowed")); break;
		default: break;
		}
	}

	// Resolution
	if (Combo_Resolution)
	{
		const FIntPoint Res = UserSettings->GetScreenResolution();
		const FString ResStr = FString::Printf(TEXT("%dx%d"), Res.X, Res.Y);
		Combo_Resolution->SetSelectedOption(ResStr);
	}

	// Quality (Overall Scalability)
	if (Combo_Quality)
	{
		const int32 QualityLevel = UserSettings->GetOverallScalabilityLevel();
		switch (QualityLevel)
		{
		case 0: Combo_Quality->SetSelectedOption(TEXT("Low")); break;
		case 1: Combo_Quality->SetSelectedOption(TEXT("Medium")); break;
		case 2: Combo_Quality->SetSelectedOption(TEXT("High")); break;
		case 3: Combo_Quality->SetSelectedOption(TEXT("Epic")); break;
		default: Combo_Quality->SetSelectedOption(TEXT("High")); break;
		}
	}

	// VSync
	if (Check_VSync)
	{
		Check_VSync->SetIsChecked(UserSettings->IsVSyncEnabled());
	}

	// FPS Limit
	if (Combo_FPSLimit)
	{
		const float FPSLimit = UserSettings->GetFrameRateLimit();
		if (FPSLimit <= 0.0f)
			Combo_FPSLimit->SetSelectedOption(TEXT("Unlimited"));
		else if (FPSLimit <= 30.0f)
			Combo_FPSLimit->SetSelectedOption(TEXT("30"));
		else if (FPSLimit <= 60.0f)
			Combo_FPSLimit->SetSelectedOption(TEXT("60"));
		else if (FPSLimit <= 120.0f)
			Combo_FPSLimit->SetSelectedOption(TEXT("120"));
		else
			Combo_FPSLimit->SetSelectedOption(TEXT("144"));
	}

	// === Sound ===
	// UE5에서는 Sound Class Volume을 사용하지만,
	// 프로토타입에서는 간단히 슬라이더 값만 표시
	if (Slider_Master)
	{
		Slider_Master->SetValue(0.8f);
		OnMasterVolumeChanged(0.8f);
	}
	if (Slider_BGM)
	{
		Slider_BGM->SetValue(0.6f);
		OnBGMVolumeChanged(0.6f);
	}
	if (Slider_SFX)
	{
		Slider_SFX->SetValue(0.8f);
		OnSFXVolumeChanged(0.8f);
	}

	// === Controls ===
	if (Slider_MouseSensitivity)
	{
		Slider_MouseSensitivity->SetValue(0.5f);
		OnMouseSensitivityChanged(0.5f);
	}
	if (Check_InvertY)
	{
		Check_InvertY->SetIsChecked(false);
	}
}

// ============================================================================
// 슬라이더 변경 콜백
// ============================================================================

void USettingsWidget::OnMasterVolumeChanged(float Value)
{
	if (Text_MasterValue)
	{
		Text_MasterValue->SetText(FText::FromString(
			FString::Printf(TEXT("%d%%"), FMath::RoundToInt(Value * 100.0f))));
	}
}

void USettingsWidget::OnBGMVolumeChanged(float Value)
{
	if (Text_BGMValue)
	{
		Text_BGMValue->SetText(FText::FromString(
			FString::Printf(TEXT("%d%%"), FMath::RoundToInt(Value * 100.0f))));
	}
}

void USettingsWidget::OnSFXVolumeChanged(float Value)
{
	if (Text_SFXValue)
	{
		Text_SFXValue->SetText(FText::FromString(
			FString::Printf(TEXT("%d%%"), FMath::RoundToInt(Value * 100.0f))));
	}
}

void USettingsWidget::OnMouseSensitivityChanged(float Value)
{
	if (Text_SensitivityValue)
	{
		Text_SensitivityValue->SetText(FText::FromString(
			FString::Printf(TEXT("%d%%"), FMath::RoundToInt(Value * 100.0f))));
	}
}

// ============================================================================
// Apply 버튼
// ============================================================================

void USettingsWidget::OnApplyClicked()
{
	ApplyGraphicsSettings();
	ApplySoundSettings();
	SaveSettings();

	UE_LOG(LogTemp, Log, TEXT("[Settings] Settings applied and saved."));
}

void USettingsWidget::ApplyGraphicsSettings()
{
	UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings();
	if (!UserSettings) return;

	// Window Mode
	if (Combo_WindowMode)
	{
		const FString Selected = Combo_WindowMode->GetSelectedOption();
		if (Selected == TEXT("Fullscreen"))
			UserSettings->SetFullscreenMode(EWindowMode::Fullscreen);
		else if (Selected == TEXT("Windowed Fullscreen"))
			UserSettings->SetFullscreenMode(EWindowMode::WindowedFullscreen);
		else
			UserSettings->SetFullscreenMode(EWindowMode::Windowed);
	}

	// Resolution
	if (Combo_Resolution)
	{
		const FString ResStr = Combo_Resolution->GetSelectedOption();
		FString WidthStr, HeightStr;
		if (ResStr.Split(TEXT("x"), &WidthStr, &HeightStr))
		{
			const int32 Width = FCString::Atoi(*WidthStr);
			const int32 Height = FCString::Atoi(*HeightStr);
			UserSettings->SetScreenResolution(FIntPoint(Width, Height));
		}
	}

	// Quality
	if (Combo_Quality)
	{
		const FString QualityStr = Combo_Quality->GetSelectedOption();
		int32 QualityLevel = 2;
		if (QualityStr == TEXT("Low"))         QualityLevel = 0;
		else if (QualityStr == TEXT("Medium"))  QualityLevel = 1;
		else if (QualityStr == TEXT("High"))    QualityLevel = 2;
		else if (QualityStr == TEXT("Epic"))    QualityLevel = 3;
		UserSettings->SetOverallScalabilityLevel(QualityLevel);
	}

	// VSync
	if (Check_VSync)
	{
		UserSettings->SetVSyncEnabled(Check_VSync->IsChecked());
	}

	// FPS Limit
	if (Combo_FPSLimit)
	{
		const FString FPSStr = Combo_FPSLimit->GetSelectedOption();
		if (FPSStr == TEXT("Unlimited"))
			UserSettings->SetFrameRateLimit(0.0f);
		else
			UserSettings->SetFrameRateLimit(FCString::Atof(*FPSStr));
	}

	// 설정 적용
	UserSettings->ApplySettings(false);
}

void USettingsWidget::ApplySoundSettings()
{
	// TODO Phase 6: Sound Class Volume 연동
	// 현재는 슬라이더 값만 저장
	UE_LOG(LogTemp, Log, TEXT("[Settings] Sound: Master=%.0f%%, BGM=%.0f%%, SFX=%.0f%%"),
		   Slider_Master ? Slider_Master->GetValue() * 100.0f : 0.0f,
		   Slider_BGM ? Slider_BGM->GetValue() * 100.0f : 0.0f,
		   Slider_SFX ? Slider_SFX->GetValue() * 100.0f : 0.0f);
}

void USettingsWidget::SaveSettings()
{
	UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings();
	if (UserSettings)
	{
		UserSettings->SaveSettings();
	}
}

// ============================================================================
// Back 버튼
// ============================================================================

void USettingsWidget::OnBackClicked()
{
	UE_LOG(LogTemp, Log, TEXT("[Settings] Back clicked."));
	RemoveFromParent();
}
