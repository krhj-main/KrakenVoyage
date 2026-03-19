// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KrakenTypes.h"
#include "GameOverWidget.generated.h"


class UButton;
class UTextBlock;
class UImage;
/**
 * 
 */
UCLASS()
class KRAKENVOYAGE_API UGameOverWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual void NativeConstruct() override;

	// ================================================================
	// BindWidget — 블루프린트에서 정확한 이름으로 매칭
	// ================================================================

	// 메인 결과 텍스트 ("CREW WINS!" / "KRAKEN WINS!")
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_ResultTitle;

	// 부제목 ("All treasures found!" / "The Kraken has awakened!")
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_ResultSubtitle;

	// 플레이어 역할 목록
	// ScrollBox나 VerticalBox 안에 동적으로 추가할 수도 있지만
	// 프로토타입에서는 단일 TextBlock에 줄바꿈으로 표시
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_PlayerRoles;

	// 게임 통계 (보물/라운드/카드 수)
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_GameStats;

	// 내 결과 ("YOU WIN!" / "YOU LOSE...")
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_PersonalResult;

	// 반투명 배경 이미지 (승리=파란, 패배=빨간)
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Img_Background;

	// 버튼
	UPROPERTY(meta = (BindWidget))
	UButton* Btn_PlayAgain;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_MainMenu;

	// ================================================================
	// 결과 데이터 설정 (PlayerController에서 호출)
	// ================================================================

	// 게임 결과를 받아서 UI에 표시
	void SetupResult(EWinCondition InWinResult, bool bPlayerWon,
		const FString& InPlayerRolesText,
		int32 TreasureFound, int32 TreasureTotal,
		int32 RoundsPlayed, int32 MaxRounds,
		int32 CardsRevealed);

protected:
	UFUNCTION()
	void OnPlayAgainClicked();

	UFUNCTION()
	void OnMainMenuClicked();
	
};
