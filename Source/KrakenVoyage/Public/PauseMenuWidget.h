#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

class UButton;
class USettingsWidget;

UCLASS()
class KRAKENVOYAGE_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Resume;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Settings;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_MainMenu;

	// 설정 위젯 클래스 (에디터에서 WBP_Settings 할당)
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<USettingsWidget> SettingsWidgetClass;

	//위젯 제거시 셋팅창 같이 닫기
	virtual void RemoveFromParent() override;

protected:
	UFUNCTION()
	void OnResumeClicked();

	UFUNCTION()
	void OnSettingsClicked();

	UFUNCTION()
	void OnMainMenuClicked();

	UPROPERTY()
	USettingsWidget* SettingsWidgetInstance = nullptr;
};