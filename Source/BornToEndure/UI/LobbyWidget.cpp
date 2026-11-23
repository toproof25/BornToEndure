/**
 * @file LobbyWidget.cpp
 * @brief 스타트 Pet 선택을 표시하고 유효한 선택이 있을 때 시작 버튼을 활성화합니다.
 */
#include "UI/LobbyWidget.h"

#include "Components/Button.h"
#include "Core/BornToEndureGameInstance.h"
#include "Core/LobbyPlayerController.h"
#include "UI/LobbyPetSelectionWidget.h"

void ULobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartButton)
	{
		StartButton->OnClicked.AddUniqueDynamic(this, &ULobbyWidget::OnStartButtonClicked);
	}

	if (PetSelectionWidget)
	{
		PetSelectionWidget->OnPetSelectionChanged.AddUObject(this, &ULobbyWidget::RefreshStartButton);
	}

	const UBornToEndureGameInstance* GameInstance = GetGameInstance<UBornToEndureGameInstance>();
	RefreshStartButton(GameInstance ? GameInstance->GetSelectedStarterPetRowName() : NAME_None);
}

void ULobbyWidget::NativeDestruct()
{
	if (StartButton)
	{
		StartButton->OnClicked.RemoveDynamic(this, &ULobbyWidget::OnStartButtonClicked);
	}

	if (PetSelectionWidget)
	{
		PetSelectionWidget->OnPetSelectionChanged.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void ULobbyWidget::RefreshStartButton(FName RowName)
{
	if (StartButton)
	{
		StartButton->SetIsEnabled(PetSelectionWidget && !RowName.IsNone());
	}
}

void ULobbyWidget::OnStartButtonClicked()
{
	if (ALobbyPlayerController* LobbyPC = Cast<ALobbyPlayerController>(GetOwningPlayer()))
	{
		LobbyPC->TravelToGameplayLevel();
	}
}
