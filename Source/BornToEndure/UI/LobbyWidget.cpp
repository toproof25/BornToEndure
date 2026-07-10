#include "UI/LobbyWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Core/LobbyPlayerController.h"


void ULobbyWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &ULobbyWidget::OnStartButtonClicked);
	}
}

void ULobbyWidget::NativeDestruct()
{
	if (StartButton)
	{
		StartButton->OnClicked.RemoveDynamic(this, &ULobbyWidget::OnStartButtonClicked);
	}
	Super::NativeDestruct();
}

void ULobbyWidget::OnStartButtonClicked()
{
	if (ALobbyPlayerController* LobbyPC = Cast<ALobbyPlayerController>(GetOwningPlayer()))
	{
		UE_LOG(LogTemp, Log, TEXT("Start Button Clicked!"));
		LobbyPC->TravelToGameplayLevel();
	}
}
