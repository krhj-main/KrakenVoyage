#include "PauseMenuWidget.h"
#include "SettingsWidget.h"
#include "KrakenGameInstance.h"
#include "KrakenPlayerController.h"
#include "Components/Button.h"

void UPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Resume)    Btn_Resume->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnResumeClicked);
	if (Btn_Settings)  Btn_Settings->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnSettingsClicked);
	if (Btn_MainMenu)  Btn_MainMenu->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnMainMenuClicked);
}

void UPauseMenuWidget::OnResumeClicked()
{
	// PlayerController의 TogglePauseMenu를 다시 호출 → 닫기
	AKrakenPlayerController* KPC = Cast<AKrakenPlayerController>(GetOwningPlayer());
	if (KPC)
	{
		KPC->TogglePauseMenu();
	}
}

void UPauseMenuWidget::OnSettingsClicked()
{
	if (SettingsWidgetInstance && SettingsWidgetInstance->IsInViewport()) return;

	if (SettingsWidgetClass)
	{
		SettingsWidgetInstance = CreateWidget<USettingsWidget>(GetOwningPlayer(), SettingsWidgetClass);
		if (SettingsWidgetInstance)
		{
			SettingsWidgetInstance->AddToViewport(30); // 일시정지(20) 위에
		}
	}
}

void UPauseMenuWidget::OnMainMenuClicked()
{
	UKrakenGameInstance* GI = Cast<UKrakenGameInstance>(GetGameInstance());
	if (GI)
	{
		GI->GoToMainMenu();
	}
}

void UPauseMenuWidget::RemoveFromParent()
{
	// Settings가 열려있으면 같이 닫기
	if (SettingsWidgetInstance && SettingsWidgetInstance->IsInViewport())
	{
		SettingsWidgetInstance->RemoveFromParent();
		SettingsWidgetInstance = nullptr;
	}

	Super::RemoveFromParent();
}