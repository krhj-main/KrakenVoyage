// MainMenuWidget.cpp

#include "MainMenuWidget.h"
#include "KrakenGameInstance.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetSystemLibrary.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼에 클릭 이벤트 바인딩
	if (Btn_Host)     Btn_Host->OnClicked.AddDynamic(this, &UMainMenuWidget::OnHostClicked);
	if (Btn_Join)     Btn_Join->OnClicked.AddDynamic(this, &UMainMenuWidget::OnJoinClicked);
	if (Btn_Settings) Btn_Settings->OnClicked.AddDynamic(this, &UMainMenuWidget::OnSettingsClicked);
	if (Btn_Quit)     Btn_Quit->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);
	if (Btn_ConnectIP) Btn_ConnectIP->OnClicked.AddDynamic(this, &UMainMenuWidget::OnConnectIPClicked);

	// IP 입력 패널은 처음에 숨김
	if (Input_IPAddress)   Input_IPAddress->SetVisibility(ESlateVisibility::Collapsed);
	if (Btn_ConnectIP)     Btn_ConnectIP->SetVisibility(ESlateVisibility::Collapsed);

	// 기본 이름 설정
	if (Input_PlayerName)
	{
		Input_PlayerName->SetText(FText::FromString(TEXT("Player")));
	}

	UE_LOG(LogTemp, Log, TEXT("[MainMenu] Widget constructed."));
}

void UMainMenuWidget::OnHostClicked()
{
	UKrakenGameInstance* GI = Cast<UKrakenGameInstance>(GetGameInstance());
	if (!GI) return;

	// 입력된 이름 가져오기
	FString PlayerName = TEXT("Host");
	if (Input_PlayerName)
	{
		PlayerName = Input_PlayerName->GetText().ToString();
		if (PlayerName.IsEmpty()) PlayerName = TEXT("Host");
	}

	UE_LOG(LogTemp, Log, TEXT("[MainMenu] Host clicked. Name: %s"), *PlayerName);
	GI->HostGame(PlayerName);
}

void UMainMenuWidget::OnJoinClicked()
{
	// IP 입력 패널 토글 (보이기/숨기기)
	if (Input_IPAddress && Btn_ConnectIP)
	{
		const bool bCurrentlyVisible = 
			(Input_IPAddress->GetVisibility() == ESlateVisibility::Visible);

		const ESlateVisibility NewVis = bCurrentlyVisible 
			? ESlateVisibility::Collapsed 
			: ESlateVisibility::Visible;

		Input_IPAddress->SetVisibility(NewVis);
		Btn_ConnectIP->SetVisibility(NewVis);

		if (!bCurrentlyVisible)
		{
			// 기본 IP 설정 (로컬 테스트용)
			Input_IPAddress->SetText(FText::FromString(TEXT("127.0.0.1")));
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[MainMenu] Join clicked - IP panel toggled."));
}

void UMainMenuWidget::OnSettingsClicked()
{
	// TODO Phase 3: 설정 화면 열기
	UE_LOG(LogTemp, Log, TEXT("[MainMenu] Settings clicked. (Not implemented yet)"));
}

void UMainMenuWidget::OnQuitClicked()
{
	UE_LOG(LogTemp, Log, TEXT("[MainMenu] Quit clicked."));
	UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
}

void UMainMenuWidget::OnConnectIPClicked()
{
	UKrakenGameInstance* GI = Cast<UKrakenGameInstance>(GetGameInstance());
	if (!GI) return;

	FString IPAddress = TEXT("127.0.0.1");
	if (Input_IPAddress)
	{
		IPAddress = Input_IPAddress->GetText().ToString();
		if (IPAddress.IsEmpty()) IPAddress = TEXT("127.0.0.1");
	}

	FString PlayerName = TEXT("Client");
	if (Input_PlayerName)
	{
		PlayerName = Input_PlayerName->GetText().ToString();
		if (PlayerName.IsEmpty()) PlayerName = TEXT("Client");
	}

	UE_LOG(LogTemp, Log, TEXT("[MainMenu] Connecting to %s as %s"), *IPAddress, *PlayerName);
	GI->JoinGame(IPAddress, PlayerName);
}
