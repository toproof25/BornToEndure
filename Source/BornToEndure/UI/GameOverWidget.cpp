#include "UI/GameOverWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Component/PlayerHealthComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Core/DefaultPlayerController.h"


void UGameOverWidget::InitializeWidget(UPlayerHealthComponent* InPlayerHealthComp)
{
	if (InPlayerHealthComp)
	{
		PlayerHealthComp = InPlayerHealthComp;
		PlayerHealthComp->OnPlayerDeath.AddUObject(this, &UGameOverWidget::OnGameOver);
	}
	this->SetVisibility(ESlateVisibility::Hidden);
}

void UGameOverWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &UGameOverWidget::OnExitButtonClicked);
	}
}

void UGameOverWidget::NativeDestruct()
{
	if (ExitButton)
	{
		ExitButton->OnClicked.RemoveDynamic(this, &UGameOverWidget::OnExitButtonClicked);
	}
	if (PlayerHealthComp)
	{
		PlayerHealthComp->OnPlayerDeath.RemoveAll(this);
	}
	Super::NativeDestruct();
}

void UGameOverWidget::OnExitButtonClicked()
{
	ADefaultPlayerController* PC = GetOwningPlayer<ADefaultPlayerController>();
	if (PC)
	{
		PC->TravelToLobbyLevel();
	}
}

void UGameOverWidget::OnGameOver()
{
	if (ExitDescriptionText)
	{
		ExitDescriptionText->SetText(FText::FromString(TEXT("Game Over!")));
	}
	if (ExitButton)
	{
		ExitButton->SetIsEnabled(true);
	}

	this->SetVisibility(ESlateVisibility::Visible);

	ADefaultPlayerController* PC = GetOwningPlayer<ADefaultPlayerController>();
	if (PC)
	{
		PC->SetUIInputMode(this, true);
	}
}
