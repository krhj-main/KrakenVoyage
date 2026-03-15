// MenuPlayerController.cpp

#include "MenuPlayerController.h"
#include "MainMenuWidget.h"

AMenuPlayerController::AMenuPlayerController()
{
	// 메인 메뉴에서는 마우스 커서 표시
	bShowMouseCursor = true;
}

void AMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 입력 모드를 UI 전용으로 (마우스 클릭이 위젯에 전달되도록)
	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	// 메인 메뉴 위젯 생성
	if (MainMenuWidgetClass)
	{
		MainMenuWidget = CreateWidget<UMainMenuWidget>(this, MainMenuWidgetClass);
		if (MainMenuWidget)
		{
			MainMenuWidget->AddToViewport(0);
			UE_LOG(LogTemp, Log, TEXT("[MenuPC] Main Menu created."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[MenuPC] MainMenuWidgetClass not set!"));
	}
}
