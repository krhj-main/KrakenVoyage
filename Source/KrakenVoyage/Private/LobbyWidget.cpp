// LobbyWidget.cpp

#include "LobbyWidget.h"
#include "KrakenGameInstance.h"
#include "KrakenGameState.h"
#include "KrakenPlayerState.h"
#include "KrakenPlayerController.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

void ULobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Ready)     Btn_Ready->OnClicked.AddDynamic(this, &ULobbyWidget::OnReadyClicked);
	if (Btn_StartGame) Btn_StartGame->OnClicked.AddDynamic(this, &ULobbyWidget::OnStartGameClicked);
	if (Btn_Leave)     Btn_Leave->OnClicked.AddDynamic(this, &ULobbyWidget::OnLeaveClicked);

	UE_LOG(LogTemp, Log, TEXT("[Lobby] Widget constructed."));
}

void ULobbyWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	UpdateLobbyDisplay();
}

void ULobbyWidget::UpdateLobbyDisplay()
{
	AGameStateBase* GS = UGameplayStatics::GetGameState(GetWorld());
	if (!GS) return;

	AKrakenPlayerController* MyPC = Cast<AKrakenPlayerController>(GetOwningPlayer());
	if (!MyPC) return;

	// ================================================================
	// 플레이어 목록 구성
	// ================================================================
	FString PlayerListStr;
	int32 PlayerCount = 0;

	for (APlayerState* PS : GS->PlayerArray)
	{
		AKrakenPlayerState* KPS = Cast<AKrakenPlayerState>(PS);
		if (!KPS) continue;

		PlayerCount++;

		// 이름
		FString EntryStr = FString::Printf(TEXT("  %s"), *KPS->GetPlayerName());

		// 호스트 표시 (인덱스 0)
		if (KPS->PlayerIndex == 0)
		{
			EntryStr += TEXT(" (Host)");
		}

		// 준비 상태
		if (KPS->bIsReady)
		{
			EntryStr += TEXT("  [Ready]");
		}

		PlayerListStr += EntryStr + TEXT("\n");
	}

	if (Text_PlayerList)
	{
		Text_PlayerList->SetText(FText::FromString(PlayerListStr));
	}

	if (Text_PlayerCount)
	{
		Text_PlayerCount->SetText(FText::FromString(
			FString::Printf(TEXT("Players: %d"), PlayerCount)));
	}

	// ================================================================
	// 준비 버튼 텍스트
	// ================================================================
	if (Text_ReadyLabel)
	{
		Text_ReadyLabel->SetText(FText::FromString(
			bIsReady ? TEXT("Cancel Ready") : TEXT("Ready")));
	}

	// ================================================================
	// 시작 버튼 (호스트만 보이게)
	// ================================================================
	if (Btn_StartGame)
	{
		const int32 MyIndex = MyPC->GetMyPlayerIndex();
		const bool bIsHost = (MyIndex == 0);
		Btn_StartGame->SetVisibility(bIsHost ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	// ================================================================
	// 상태 메시지
	// ================================================================
	if (Text_Status)
	{
		AKrakenGameState* KGS = Cast<AKrakenGameState>(GS);
		if (KGS && KGS->CurrentPhase == EKrakenGamePhase::WaitingForPlayers)
		{
			Text_Status->SetText(FText::FromString(TEXT("Waiting for players...")));
		}
		else if (KGS)
		{
			Text_Status->SetText(FText::FromString(TEXT("Game in progress!")));
		}
	}
}

void ULobbyWidget::OnReadyClicked()
{
	bIsReady = !bIsReady;

	AKrakenPlayerController* MyPC = Cast<AKrakenPlayerController>(GetOwningPlayer());
	if (MyPC)
	{
		MyPC->ServerToggleReady();
	}

	UE_LOG(LogTemp, Log, TEXT("[Lobby] Ready toggled: %s"), bIsReady ? TEXT("Ready") : TEXT("Not Ready"));
}

void ULobbyWidget::OnStartGameClicked()
{
	AKrakenPlayerController* MyPC = Cast<AKrakenPlayerController>(GetOwningPlayer());
	if (MyPC)
	{
		MyPC->ServerRequestStartGame();
	}

	UE_LOG(LogTemp, Log, TEXT("[Lobby] Start Game requested."));
}

void ULobbyWidget::OnLeaveClicked()
{
	UKrakenGameInstance* GI = Cast<UKrakenGameInstance>(GetGameInstance());
	if (GI)
	{
		GI->GoToMainMenu();
	}

	UE_LOG(LogTemp, Log, TEXT("[Lobby] Leave clicked."));
}
