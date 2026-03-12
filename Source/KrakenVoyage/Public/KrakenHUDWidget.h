// KrakenHUDWidget.h
// ============================================================================
// HUD 위젯 - BindWidget 방식으로 C++에서 직접 위젯 제어
// ============================================================================
//
// BindWidget 패턴:
//   C++ 헤더에서 UPROPERTY(meta=(BindWidget)) 로 위젯 포인터를 선언하면,
//   블루프린트 위젯에서 "같은 이름"의 위젯을 자동으로 연결해줌.
//
//   장점: 블루프린트에서 바인딩 설정이 전혀 필요 없음!
//         C++ NativeTick에서 SetText(), SetVisibility()를 직접 호출
//
//   규칙: C++ 변수 이름 = 블루프린트 위젯 이름 (정확히 일치해야 함)
//
// ============================================================================

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KrakenTypes.h"
#include "KrakenHUDWidget.generated.h"

class UTextBlock;
class AKrakenGameState;
class AKrakenPlayerController;

UCLASS()
class KRAKENVOYAGE_API UKrakenHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// ========================================================================
	// BindWidget: 블루프린트의 위젯과 자동 연결
	// ========================================================================
	// meta=(BindWidget) → 블루프린트에 같은 이름의 위젯이 반드시 있어야 함
	// meta=(BindWidgetOptional) → 없어도 에러 안 남 (nullptr)
	
	// --- 좌측 상단: 게임 정보 ---
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Phase;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Round;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Turn;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Treasure;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Revealed;

	// --- 좌측 하단: 내 정보 ---
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_MyRole;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_MyCards;

	// --- 상단 중앙: 턴 알림 ---
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_ActionHolder;

	// --- 하단 중앙: 상호작용 힌트 ---
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_InteractionHint;

	// --- 중앙: 카드 공개 알림 ---
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_LastRevealed;

	// --- 중앙: 게임 결과 ---
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_GameResult;

protected:
	void UpdateHUDData();
	FString GetPhaseDisplayName(EKrakenGamePhase Phase) const;

	// 헬퍼: 텍스트 + 색상 + Visibility를 한번에 설정
	void SetTextSafe(UTextBlock* TextBlock, const FString& InText,
		FLinearColor Color = FLinearColor::White,
		ESlateVisibility InVisibility = ESlateVisibility::HitTestInvisible);

	UPROPERTY()
	AKrakenPlayerController* CachedPC = nullptr;

	UPROPERTY()
	AKrakenGameState* CachedGS = nullptr;
};
