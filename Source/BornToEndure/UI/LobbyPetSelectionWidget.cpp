/**
 * @file LobbyPetSelectionWidget.cpp
 * @brief Pet 테이블의 행을 선택 카드로 표시하고 게임 인스턴스에 저장된 선택을 반영합니다.
 */
#include "UI/LobbyPetSelectionWidget.h"

#include "Components/HorizontalBox.h"
#include "Core/BornToEndureGameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Subsystem/ItemPoolSubsystem.h"
#include "UI/LobbyPetEntryWidget.h"

void ULobbyPetSelectionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BuildPetList();

	const UBornToEndureGameInstance* GameInstance = GetGameInstance<UBornToEndureGameInstance>();
	RefreshSelection(GameInstance ? GameInstance->GetSelectedStarterPetRowName() : NAME_None);
}

void ULobbyPetSelectionWidget::NativeDestruct()
{
	if (PetListBox)
	{
		for (UWidget* Child : PetListBox->GetAllChildren())
		{
			if (ULobbyPetEntryWidget* Entry = Cast<ULobbyPetEntryWidget>(Child))
			{
				Entry->OnPetClicked.RemoveAll(this);
			}
		}

		PetListBox->ClearChildren();
	}

	Super::NativeDestruct();
}

void ULobbyPetSelectionWidget::BuildPetList()
{
	APlayerController* OwningPlayer = GetOwningPlayer();
	UGameInstance* GameInstance = GetGameInstance();
	UItemPoolSubsystem* ItemPool = GameInstance ? GameInstance->GetSubsystem<UItemPoolSubsystem>() : nullptr;
	if (!PetListBox || !PetEntryWidgetClass || !OwningPlayer || !ItemPool)
	{
		return;
	}

	for (const FName RowName : ItemPool->GetPetRowNames())
	{
		const FPetDataRow* Row = ItemPool->GetPetDataRowByID(RowName);
		if (!Row)
		{
			continue;
		}

		ULobbyPetEntryWidget* Entry = CreateWidget<ULobbyPetEntryWidget>(OwningPlayer, PetEntryWidgetClass);
		if (Entry)
		{
			Entry->InitializePet(RowName, *Row);
			Entry->OnPetClicked.AddUObject(this, &ULobbyPetSelectionWidget::HandlePetEntryClicked);
			PetListBox->AddChildToHorizontalBox(Entry);
		}
	}
}

void ULobbyPetSelectionWidget::HandlePetEntryClicked(FName RowName)
{
	UBornToEndureGameInstance* GameInstance = GetGameInstance<UBornToEndureGameInstance>();
	if (!GameInstance || !GameInstance->SelectStarterPet(RowName))
	{
		return;
	}

	RefreshSelection(RowName);
	OnPetSelectionChanged.Broadcast(RowName);
}

void ULobbyPetSelectionWidget::RefreshSelection(FName RowName)
{
	if (!PetListBox)
	{
		return;
	}

	for (UWidget* Child : PetListBox->GetAllChildren())
	{
		if (ULobbyPetEntryWidget* Entry = Cast<ULobbyPetEntryWidget>(Child))
		{
			Entry->SetSelected(Entry->GetPetRowName() == RowName);
		}
	}
}
