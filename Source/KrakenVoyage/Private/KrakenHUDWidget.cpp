// KrakenHUDWidget.cpp

#include "KrakenHUDWidget.h"
#include "KrakenGameState.h"
#include "KrakenPlayerController.h"
#include "KrakenPlayerState.h"
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

	SetTextSafe(Text_InteractionHint, TEXT(""), FLinearColor::White, ESlateVisibility::Hidden);
	SetTextSafe(Text_LastRevealed, TEXT(""), FLinearColor::White, ESlateVisibility::Hidden);
	SetTextSafe(Text_GameResult, TEXT(""), FLinearColor::White, ESlateVisibility::Hidden);
}

void UKrakenHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	UpdateHUDData();
}

void UKrakenHUDWidget::SetTextSafe(UTextBlock* TextBlock, const FString& InText,
								   FLinearColor Color, ESlateVisibility InVisibility)
{
	if (!TextBlock) return;
	TextBlock->SetText(FText::FromString(InText));
	TextBlock->SetColorAndOpacity(FSlateColor(Color));
	TextBlock->SetVisibility(InVisibility);
}

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

	// 내 플레이어 인덱스 (멀티플레이어에서 각자 다름)
	const int32 MyIndex = CachedPC->GetMyPlayerIndex();

	// ================================================================
	// 좌측 상단: 게임 정보
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
	// 좌측 하단: 내 정보
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
		// ★ 핵심: MyIndex와 비교하여 내 턴인지 판단
		const bool bIsMyTurn = (CachedGS->ActionHolderPlayerIndex == MyIndex);
		if (bIsMyTurn)
		{
			SetTextSafe(Text_ActionHolder, TEXT("YOUR TURN - Select a box! (then ConfirmReveal)"), YellowColor);
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
	// 하단 중앙: 상호작용 힌트
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
	// 중앙: 카드 공개 알림
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
	// 중앙: 게임 결과
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

		if (Text_LastRevealed) Text_LastRevealed->SetVisibility(ESlateVisibility::Hidden);
		SetTextSafe(Text_ActionHolder, TEXT(""));
	}
	else
	{
		if (Text_GameResult)
		{
			Text_GameResult->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

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
