// MainMenuWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UEditableTextBox;
class UTextBlock;
class USettingsWidget;

UCLASS()
class KRAKENVOYAGE_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// ========================================================================
	// BindWidget
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

	UPROPERTY(meta = (BindWidgetOptional))
	UEditableTextBox* Input_IPAddress;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_ConnectIP;

	// ========================================================================
	// 설정 위젯 클래스 (에디터에서 할당)
	// ========================================================================

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<USettingsWidget> SettingsWidgetClass;

protected:
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

	UPROPERTY()
	USettingsWidget* SettingsWidgetInstance = nullptr;
};
