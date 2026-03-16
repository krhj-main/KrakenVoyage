// MainMenuWidget.cpp

#include "MainMenuWidget.h"
#include "SettingsWidget.h"
#include "KrakenGameInstance.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetSystemLibrary.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Host)       Btn_Host->OnClicked.AddDynamic(this, &UMainMenuWidget::OnHostClicked);
	if (Btn_Join)       Btn_Join->OnClicked.AddDynamic(this, &UMainMenuWidget::OnJoinClicked);
	if (Btn_Settings)   Btn_Settings->OnClicked.AddDynamic(this, &UMainMenuWidget::OnSettingsClicked);
	if (Btn_Quit)       Btn_Quit->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);
	if (Btn_ConnectIP)  Btn_ConnectIP->OnClicked.AddDynamic(this, &UMainMenuWidget::OnConnectIPClicked);

	if (Input_IPAddress) Input_IPAddress->SetVisibility(ESlateVisibility::Collapsed);
	if (Btn_ConnectIP)   Btn_ConnectIP->SetVisibility(ESlateVisibility::Collapsed);

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

	FString PlayerName = TEXT("Host");
	if (Input_PlayerName)
	{
		PlayerName = Input_PlayerName->GetText().ToString();
		if (PlayerName.IsEmpty()) PlayerName = TEXT("Host");
	}

	UE_LOG(LogTemp, Log, TEXT("[MainMenu] Host clicked. Name: %s"), *PlayerName);

	// HostGame(이름, 최대인원)
	// 기본 6명, 나중에 로비 설정에서 변경 가능
	GI->HostGame(PlayerName, 6);
}

void UMainMenuWidget::OnJoinClicked()
{
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
			Input_IPAddress->SetText(FText::FromString(TEXT("127.0.0.1")));
		}
	}
}

void UMainMenuWidget::OnSettingsClicked()
{
	UE_LOG(LogTemp, Log, TEXT("[MainMenu] Settings clicked."));

	if (SettingsWidgetInstance && SettingsWidgetInstance->IsInViewport())
	{
		return;
	}

	if (SettingsWidgetClass)
	{
		SettingsWidgetInstance = CreateWidget<USettingsWidget>(GetOwningPlayer(), SettingsWidgetClass);
		if (SettingsWidgetInstance)
		{
			SettingsWidgetInstance->AddToViewport(10);
		}
	}
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
	GI->JoinByIP(IPAddress, PlayerName);
}
