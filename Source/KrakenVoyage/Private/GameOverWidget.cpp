#include "GameOverWidget.h"
#include "KrakenGameInstance.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"

void UGameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 바인딩
	if (Btn_PlayAgain)
	{
		Btn_PlayAgain->OnClicked.AddDynamic(this, &UGameOverWidget::OnPlayAgainClicked);
	}
	if (Btn_MainMenu)
	{
		Btn_MainMenu->OnClicked.AddDynamic(this, &UGameOverWidget::OnMainMenuClicked);
	}

	UE_LOG(LogTemp, Log, TEXT("[GameOver] Widget constructed."));
}

// ============================================================
// 결과 데이터 설정
// ============================================================
void UGameOverWidget::SetupResult(EWinCondition InWinResult, bool bPlayerWon,
	const FString& InPlayerRolesText,
	int32 TreasureFound, int32 TreasureTotal,
	int32 RoundsPlayed, int32 MaxRounds,
	int32 CardsRevealed)
{
	// ── 메인 타이틀 ──
	// 승리 조건에 따라 다른 텍스트와 색상
	FString TitleText;
	FString SubtitleText;
	FLinearColor TitleColor;

	switch (InWinResult)
	{
	case EWinCondition::CrewFoundAllTreasure:
		TitleText = TEXT("CREW WINS!");
		SubtitleText = TEXT("All treasures have been found!");
		TitleColor = FLinearColor(1.0f, 0.84f, 0.0f); // 금색
		break;

	case EWinCondition::KrakenRevealed:
		TitleText = TEXT("KRAKEN WINS!");
		SubtitleText = TEXT("The Kraken has been unleashed!");
		TitleColor = FLinearColor(0.8f, 0.1f, 0.8f); // 보라색
		break;

	case EWinCondition::TimeRanOut:
		TitleText = TEXT("KRAKEN WINS!");
		SubtitleText = TEXT("Time ran out! The crew failed...");
		TitleColor = FLinearColor(0.8f, 0.1f, 0.8f);
		break;

	default:
		TitleText = TEXT("GAME OVER");
		SubtitleText = TEXT("");
		TitleColor = FLinearColor::White;
		break;
	}

	if (Text_ResultTitle)
	{
		Text_ResultTitle->SetText(FText::FromString(TitleText));
		Text_ResultTitle->SetColorAndOpacity(FSlateColor(TitleColor));
	}

	if (Text_ResultSubtitle)
	{
		Text_ResultSubtitle->SetText(FText::FromString(SubtitleText));
	}

	// ── 내 결과 ──
	if (Text_PersonalResult)
	{
		if (bPlayerWon)
		{
			Text_PersonalResult->SetText(FText::FromString(TEXT("YOU WIN!")));
			Text_PersonalResult->SetColorAndOpacity(
				FSlateColor(FLinearColor(0.2f, 1.0f, 0.3f))); // 초록
		}
		else
		{
			Text_PersonalResult->SetText(FText::FromString(TEXT("YOU LOSE...")));
			Text_PersonalResult->SetColorAndOpacity(
				FSlateColor(FLinearColor(1.0f, 0.2f, 0.2f))); // 빨강
		}
	}

	// ── 플레이어 역할 목록 ──
	// GameMode에서 만든 문자열을 그대로 표시
	// 예: "Player 0: Crew\nPlayer 1: KRAKEN\nPlayer 2: Crew"
	if (Text_PlayerRoles)
	{
		Text_PlayerRoles->SetText(FText::FromString(InPlayerRolesText));
	}

	// ── 게임 통계 ──
	if (Text_GameStats)
	{
		const FString StatsText = FString::Printf(
			TEXT("Treasures: %d / %d\nRounds: %d / %d\nCards Revealed: %d"),
			TreasureFound, TreasureTotal,
			RoundsPlayed, MaxRounds,
			CardsRevealed);
		Text_GameStats->SetText(FText::FromString(StatsText));
	}

	// ── 배경색 ──
	// UImage의 ColorAndOpacity로 반투명 배경색 변경
	if (Img_Background)
	{
		FLinearColor BgColor;
		if (bPlayerWon)
		{
			BgColor = FLinearColor(0.0f, 0.1f, 0.3f, 0.85f); // 어두운 파란 (승리)
		}
		else
		{
			BgColor = FLinearColor(0.3f, 0.0f, 0.0f, 0.85f); // 어두운 빨강 (패배)
		}
		Img_Background->SetColorAndOpacity(BgColor);
	}
}

// ============================================================
// Play Again → 로비로 돌아가기
// ============================================================
void UGameOverWidget::OnPlayAgainClicked()
{
	UE_LOG(LogTemp, Log, TEXT("[GameOver] Play Again clicked."));

	// 호스트라면: 게임을 리셋하고 로비로
	// 클라이언트라면: 서버에 요청
	// 현재 프로토타입: 같은 맵을 다시 로드 (ServerTravel)
	APlayerController* PC = GetOwningPlayer();
	if (PC && PC->HasAuthority())
	{
		// 호스트: ServerTravel로 같은 맵 다시 로드
		// "?listen"을 붙여야 Listen Server 유지
		UWorld* World = GetWorld();
		if (World)
		{
			// 현재 맵 이름 가져오기
			const FString MapName = World->GetMapName();
			// "UEDPIE_0_" 접두사 제거
			FString CleanMapName = MapName;
			CleanMapName.RemoveFromStart(TEXT("UEDPIE_0_"));

			World->ServerTravel(CleanMapName + TEXT("?listen"));
			UE_LOG(LogTemp, Log, TEXT("[GameOver] ServerTravel to %s"), *CleanMapName);
		}
	}
	else
	{
		// 클라이언트: 호스트가 재시작하길 기다림
		// TODO: ServerRPC로 재시작 요청
		UE_LOG(LogTemp, Log, TEXT("[GameOver] Waiting for host to restart..."));
	}
}

// ============================================================
// Main Menu → 메인 메뉴로 복귀
// ============================================================
void UGameOverWidget::OnMainMenuClicked()
{
	UE_LOG(LogTemp, Log, TEXT("[GameOver] Main Menu clicked."));

	UKrakenGameInstance* GI = Cast<UKrakenGameInstance>(GetGameInstance());
	if (GI)
	{
		// GoToMainMenu: 세션 파괴 + 메인 메뉴 레벨로 이동
		GI->GoToMainMenu();
	}
}