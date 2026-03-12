// KrakenHUDWidget.cpp

#include "KrakenHUDWidget.h"
#include "KrakenGameState.h"
#include "KrakenPlayerController.h"
#include "KrakenCharacter.h"
#include "ExplorationBox.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UKrakenHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CachedPC = Cast<AKrakenPlayerController>(GetOwningPlayer());
	CachedGS = Cast<AKrakenGameState>(UGameplayStatics::GetGameState(GetWorld()));

	UE_LOG(LogTemp, Log, TEXT("[HUD] Constructed. PC=%s, GS=%s"),
		   CachedPC ? TEXT("OK") : TEXT("NULL"),
		   CachedGS ? TEXT("OK") : TEXT("NULL"));

	// 초기에 조건부 위젯들은 숨김
	SetTextSafe(Text_InteractionHint, TEXT(""), FLinearColor::White, ESlateVisibility::Hidden);
	SetTextSafe(Text_LastRevealed, TEXT(""), FLinearColor::White, ESlateVisibility::Hidden);
	SetTextSafe(Text_GameResult, TEXT(""), FLinearColor::White, ESlateVisibility::Hidden);
}

void UKrakenHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	UpdateHUDData();
}

// ============================================================================
// 헬퍼: 안전하게 텍스트/색상/Visibility 설정
// ============================================================================
void UKrakenHUDWidget::SetTextSafe(UTextBlock* TextBlock, const FString& InText,
	FLinearColor Color, ESlateVisibility InVisibility)
{
	if (!TextBlock) return;
	TextBlock->SetText(FText::FromString(InText));
	TextBlock->SetColorAndOpacity(FSlateColor(Color));
	TextBlock->SetVisibility(InVisibility);
}

// ============================================================================
// 매 틱 데이터 업데이트
// ============================================================================
void UKrakenHUDWidget::UpdateHUDData()
{
	if (!CachedGS)
	{
		CachedGS = Cast<AKrakenGameState>(UGameplayStatics::GetGameState(GetWorld()));
	}
	if (!CachedPC)
	{
		CachedPC = Cast<AKrakenPlayerController>(GetOwningPlayer());
	}
	if (!CachedGS || !CachedPC) return;

	const FLinearColor WhiteColor(1.0f, 1.0f, 1.0f);
	const FLinearColor GoldColor(1.0f, 0.84f, 0.0f);
	const FLinearColor PurpleColor(0.8f, 0.1f, 0.8f);
	const FLinearColor GrayColor(0.6f, 0.6f, 0.6f);
	const FLinearColor GreenColor(0.2f, 1.0f, 0.3f);
	const FLinearColor RedColor(1.0f, 0.2f, 0.2f);
	const FLinearColor CyanColor(0.2f, 0.8f, 1.0f);
	const FLinearColor YellowColor(1.0f, 1.0f, 0.3f);

	// ================================================================
	// 좌측 상단: 게임 정보 (항상 표시)
	// ================================================================
	SetTextSafe(Text_Phase, GetPhaseDisplayName(CachedGS->CurrentPhase), YellowColor);

	if (CachedGS->CurrentRound > 0)
	{
		SetTextSafe(Text_Round,
			FString::Printf(TEXT("Round: %d / %d"), CachedGS->CurrentRound, CachedGS->MaxRounds),
			WhiteColor);
		SetTextSafe(Text_Turn,
			FString::Printf(TEXT("Turn: %d / %d"), CachedGS->CurrentTurnInRound + 1, CachedGS->TotalTreasureCount),
			WhiteColor);
	}
	else
	{
		SetTextSafe(Text_Round, TEXT("Round: -"), GrayColor);
		SetTextSafe(Text_Turn, TEXT("Turn: -"), GrayColor);
	}

	SetTextSafe(Text_Treasure,
		FString::Printf(TEXT("Treasure: %d / %d"), CachedGS->RevealedTreasureCount, CachedGS->TotalTreasureCount),
		GoldColor);

	SetTextSafe(Text_Revealed,
		FString::Printf(TEXT("Cards Revealed: %d"), CachedGS->RevealedCards.Num()),
		WhiteColor);

	// ================================================================
	// 좌측 하단: 내 정보 (항상 표시)
	// ================================================================
	if (CachedPC->MyRole == EPlayerRole::Crew)
	{
		SetTextSafe(Text_MyRole, TEXT("Role: CREW"), CyanColor);
	}
	else if (CachedPC->MyRole == EPlayerRole::Kraken)
	{
		SetTextSafe(Text_MyRole, TEXT("Role: KRAKEN"), PurpleColor);
	}
	else
	{
		SetTextSafe(Text_MyRole, TEXT("Role: ???"), GrayColor);
	}

	if (CachedPC->MyRole != EPlayerRole::None)
	{
		FString KrakenStr = CachedPC->bMyHasKraken ? TEXT(" | KRAKEN!") : TEXT("");
		SetTextSafe(Text_MyCards,
			FString::Printf(TEXT("Empty:%d  Treasure:%d%s"),
				CachedPC->MyEmptyCount, CachedPC->MyTreasureCount, *KrakenStr),
			WhiteColor);
	}

	// ================================================================
	// 상단 중앙: 턴 알림
	// ================================================================
	if (CachedGS->CurrentPhase == EKrakenGamePhase::Discussion)
	{
		// 디버그 모드에서는 항상 Player 0이 플레이어
		if (CachedGS->ActionHolderPlayerIndex == 0)
		{
			SetTextSafe(Text_ActionHolder, TEXT("YOUR TURN - Select a box!"), YellowColor);
		}
		else
		{
			SetTextSafe(Text_ActionHolder,
				FString::Printf(TEXT("Player %d is choosing..."), CachedGS->ActionHolderPlayerIndex),
				GrayColor);
		}
	}
	else if (CachedGS->CurrentPhase == EKrakenGamePhase::RoleReveal)
	{
		SetTextSafe(Text_ActionHolder, TEXT("Check your role..."), CyanColor);
	}
	else if (CachedGS->CurrentPhase == EKrakenGamePhase::CardCheck)
	{
		SetTextSafe(Text_ActionHolder, TEXT("Memorize your cards!"), CyanColor);
	}
	else if (CachedGS->CurrentPhase == EKrakenGamePhase::RoundTransition)
	{
		SetTextSafe(Text_ActionHolder, TEXT("Cards are being reshuffled..."), YellowColor);
	}
	else
	{
		SetTextSafe(Text_ActionHolder, TEXT(""));
	}

	// ================================================================
	// 하단 중앙: 상호작용 힌트 (상자 바라볼 때만)
	// ================================================================
	AKrakenCharacter* MyChar = Cast<AKrakenCharacter>(CachedPC->GetPawn());
	if (MyChar && MyChar->CurrentInteractTarget)
	{
		AExplorationBox* Box = MyChar->CurrentInteractTarget;
		if (!Box->bIsRevealed)
		{
			SetTextSafe(Text_InteractionHint,
				FString::Printf(TEXT("[E] Open Box (Player %d, #%d)"),
					Box->OwnerPlayerIndex, Box->BoxIndex + 1),
				WhiteColor, ESlateVisibility::HitTestInvisible);
		}
		else
		{
			SetTextSafe(Text_InteractionHint, TEXT("(Already opened)"),
				GrayColor, ESlateVisibility::HitTestInvisible);
		}
	}
	else
	{
		if (Text_InteractionHint)
		{
			Text_InteractionHint->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	// ================================================================
	// 중앙: 카드 공개 알림 (공개 직후만)
	// ================================================================
	if (CachedGS->CurrentPhase == EKrakenGamePhase::Reveal ||
		CachedGS->CurrentPhase == EKrakenGamePhase::Discussion)
	{
		if (CachedGS->RevealedCards.Num() > 0)
		{
			const FRevealedCardInfo& Last = CachedGS->RevealedCards.Last();
			if (Last.CardType == ECardType::Treasure)
			{
				SetTextSafe(Text_LastRevealed, TEXT("TREASURE FOUND!"),
					GoldColor, ESlateVisibility::HitTestInvisible);
			}
			else if (Last.CardType == ECardType::Kraken)
			{
				SetTextSafe(Text_LastRevealed, TEXT("KRAKEN AWAKENED!"),
					PurpleColor, ESlateVisibility::HitTestInvisible);
			}
			else
			{
				SetTextSafe(Text_LastRevealed, TEXT("Empty..."),
					GrayColor, ESlateVisibility::HitTestInvisible);
			}
		}
	}
	else
	{
		if (Text_LastRevealed)
		{
			Text_LastRevealed->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	// ================================================================
	// 중앙: 게임 결과 (게임 종료 시만)
	// ================================================================
	if (CachedGS->CurrentPhase == EKrakenGamePhase::GameOver)
	{
		FString ResultText;
		FLinearColor ResultColor;

		const bool bImCrew = (CachedPC->MyRole == EPlayerRole::Crew);

		if (CachedGS->WinResult == EWinCondition::CrewFoundAllTreasure)
		{
			ResultText = TEXT("CREW WINS!\nAll treasures found!");
			ResultColor = bImCrew ? GreenColor : RedColor;
		}
		else if (CachedGS->WinResult == EWinCondition::KrakenRevealed)
		{
			ResultText = TEXT("KRAKEN WINS!\nThe Kraken has been unleashed!");
			ResultColor = bImCrew ? RedColor : GreenColor;
		}
		else if (CachedGS->WinResult == EWinCondition::TimeRanOut)
		{
			ResultText = TEXT("KRAKEN WINS!\nTime ran out!");
			ResultColor = bImCrew ? RedColor : GreenColor;
		}

		SetTextSafe(Text_GameResult, ResultText, ResultColor, ESlateVisibility::HitTestInvisible);

		// 게임 종료 시 다른 알림 숨기기
		if (Text_LastRevealed) Text_LastRevealed->SetVisibility(ESlateVisibility::Hidden);
		if (Text_ActionHolder) SetTextSafe(Text_ActionHolder, TEXT(""));
	}
	else
	{
		if (Text_GameResult)
		{
			Text_GameResult->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

// ============================================================================
// 페이즈 이름
// ============================================================================
FString UKrakenHUDWidget::GetPhaseDisplayName(EKrakenGamePhase Phase) const
{
	switch (Phase)
	{
	case EKrakenGamePhase::WaitingForPlayers: return TEXT("Waiting for Players");
	case EKrakenGamePhase::RoleReveal:        return TEXT("Role Reveal");
	case EKrakenGamePhase::CardCheck:         return TEXT("Check Your Cards");
	case EKrakenGamePhase::Discussion:        return TEXT("Discussion");
	case EKrakenGamePhase::Reveal:            return TEXT("Revealing...");
	case EKrakenGamePhase::RoundTransition:   return TEXT("Next Round");
	case EKrakenGamePhase::GameOver:          return TEXT("Game Over");
	default:                                  return TEXT("Unknown");
	}
}
