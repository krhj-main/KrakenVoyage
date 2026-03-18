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

	// ============================================================
// ★ Phase 5: 보이스 표시
// ============================================================
	if (Text_VoiceIndicator)
	{
		// 내 PlayerState에서 bIsTalking 확인
		APlayerController* PC = GetOwningPlayer();
		if (PC)
		{
			AKrakenPlayerState* MyPS = PC->GetPlayerState<AKrakenPlayerState>();
			if (MyPS && MyPS->bIsTalking)
			{
				Text_VoiceIndicator->SetText(FText::FromString(TEXT("MIC ON")));
				Text_VoiceIndicator->SetColorAndOpacity(
					FSlateColor(FLinearColor(0.0f, 1.0f, 0.0f))); // 초록색
				Text_VoiceIndicator->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				Text_VoiceIndicator->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}

	// ============================================================
// ★ Phase 6: 토론 타이머 표시
// ============================================================
	if (CachedGS && Text_Timer)
	{
		if (CachedGS->CurrentPhase == EKrakenGamePhase::Discussion)
		{
			// GetServerWorldTimeSeconds: 서버와 동기화된 시간
			const float Remaining = CachedGS->PhaseEndTime - GetWorld()->GetTimeSeconds();
			const int32 Seconds = FMath::Max(0, FMath::CeilToInt(Remaining));
			const int32 Min = Seconds / 60;
			const int32 Sec = Seconds % 60;

			Text_Timer->SetText(FText::FromString(
				FString::Printf(TEXT("%d:%02d"), Min, Sec)));
			Text_Timer->SetVisibility(ESlateVisibility::Visible);

			// 색상: 30초+ 흰색, 10~30 노란색, 10초 미만 빨간색
			FSlateColor TimerColor;
			if (Seconds > 30)
				TimerColor = FSlateColor(FLinearColor::White);
			else if (Seconds > 10)
				TimerColor = FSlateColor(FLinearColor(1.0f, 1.0f, 0.0f));
			else
			{
				// 깜빡임
				const bool bBlink = FMath::Fmod(GetWorld()->GetTimeSeconds(), 1.0f) < 0.5f;
				TimerColor = bBlink
					? FSlateColor(FLinearColor::Red)
					: FSlateColor(FLinearColor(0.5f, 0.0f, 0.0f));
			}
			Text_Timer->SetColorAndOpacity(TimerColor);
		}
		else
		{
			Text_Timer->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	// ============================================================
	// ★ Phase 6: 행동 안내 텍스트
	// ============================================================
	if (CachedGS && Text_PhaseGuide)
	{
		FString GuideStr;
		FLinearColor GuideColor = FLinearColor::White;

		switch (CachedGS->CurrentPhase)
		{
		case EKrakenGamePhase::RoleReveal:
			GuideStr = TEXT("Your role has been assigned!");
			GuideColor = FLinearColor(0.0f, 1.0f, 1.0f); // Cyan
			break;

		case EKrakenGamePhase::CardCheck:
			GuideStr = TEXT("Memorize your cards!");
			GuideColor = FLinearColor(0.0f, 1.0f, 1.0f);
			break;

		case EKrakenGamePhase::Discussion:
		{
			int32 MyIndex = CachedPC->GetMyPlayerIndex();
			const bool bIsMyTurn = (CachedGS->ActionHolderPlayerIndex == MyIndex);
			if (bIsMyTurn)
			{
				if (CachedGS->PendingSelectionPlayerIndex < 0)
					GuideStr = TEXT("[E] Select a box    |    YOUR TURN");
				else
					GuideStr = TEXT("[F] Confirm    |    [E] Change selection");
				GuideColor = FLinearColor(1.0f, 1.0f, 0.0f); // Yellow
			}
			else
			{
				GuideStr = FString::Printf(TEXT("Player %d is choosing..."),
					CachedGS->ActionHolderPlayerIndex);
				GuideColor = FLinearColor(0.5f, 0.5f, 0.5f); // Gray
			}
			break;
		}

		case EKrakenGamePhase::RoundTransition:
			GuideStr = TEXT("Cards are being reshuffled...");
			GuideColor = FLinearColor(1.0f, 1.0f, 0.0f);
			break;

		case EKrakenGamePhase::GameOver:
			GuideStr = TEXT("");
			break;

		default:
			GuideStr = TEXT("");
			break;
		}

		Text_PhaseGuide->SetText(FText::FromString(GuideStr));
		Text_PhaseGuide->SetColorAndOpacity(FSlateColor(GuideColor));
		Text_PhaseGuide->SetVisibility(
			GuideStr.IsEmpty() ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
	}
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
