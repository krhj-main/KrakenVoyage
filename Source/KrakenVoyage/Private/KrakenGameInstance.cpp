// KrakenGameInstance.cpp

#include "KrakenGameInstance.h"
#include "Kismet/GameplayStatics.h"

UKrakenGameInstance::UKrakenGameInstance()
{
}

void UKrakenGameInstance::GoToMainMenu()
{
	// 기존 세션 정리
	APlayerController* PC = GetFirstLocalPlayerController();
	if (PC)
	{
		// 서버에서 나가기
		PC->ClientTravel(MainMenuMapPath, TRAVEL_Absolute);
	}

	UE_LOG(LogTemp, Log, TEXT("[GameInstance] Returning to Main Menu"));
}

void UKrakenGameInstance::HostGame(const FString& PlayerName)
{
	LocalPlayerName = PlayerName;

	// Listen Server로 게임 맵 열기
	// "?listen" 옵션이 이 클라이언트를 Listen Server로 만듦
	// 다른 플레이어가 이 서버의 IP로 접속할 수 있게 됨
	UWorld* World = GetWorld();
	if (World)
	{
		const FString TravelURL = GameMapPath + TEXT("?listen");
		World->ServerTravel(TravelURL);
		UE_LOG(LogTemp, Log, TEXT("[GameInstance] Hosting game: %s"), *TravelURL);
	}
}

void UKrakenGameInstance::JoinGame(const FString& IPAddress, const FString& PlayerName)
{
	LocalPlayerName = PlayerName;

	// 해당 IP의 서버에 접속
	APlayerController* PC = GetFirstLocalPlayerController();
	if (PC)
	{
		PC->ClientTravel(IPAddress, TRAVEL_Absolute);
		UE_LOG(LogTemp, Log, TEXT("[GameInstance] Joining game at: %s"), *IPAddress);
	}
}
