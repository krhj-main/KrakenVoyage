// MenuPlayerController.h
// ============================================================================
// 메인 메뉴용 플레이어 컨트롤러
// ============================================================================
// 메인 메뉴에서는 마우스 커서가 보여야 하고,
// 캐릭터 이동이 아닌 UI 클릭이 입력의 주체
// 인게임의 KrakenPlayerController와 분리

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MenuPlayerController.generated.h"

class UMainMenuWidget;

UCLASS()
class KRAKENVOYAGE_API AMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMenuPlayerController();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UMainMenuWidget> MainMenuWidgetClass;

	UPROPERTY()
	UMainMenuWidget* MainMenuWidget = nullptr;
};
