/**
 * @file LobbyPetEntryWidget.cpp
 * @brief Pet 행으로 카드 표현을 초기화하고 선택 입력을 목록에 전달합니다.
 */
#include "UI/LobbyPetEntryWidget.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/DataTableRow/PetDataRow.h"
#include "Engine/Texture2D.h"

void ULobbyPetEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (PetButton)
	{
		PetButton->OnClicked.AddUniqueDynamic(this, &ULobbyPetEntryWidget::HandlePetButtonClicked);
	}
}

void ULobbyPetEntryWidget::InitializePet(FName RowName, const FPetDataRow& Row)
{
	if (!PetButton || !PetIcon || !PetNameText || !SelectionBorder)
	{
		SetIsEnabled(false);
		return;
	}

	PetRowName = RowName;
	PetNameText->SetText(Row.Name);
	PetButton->SetIsEnabled(!RowName.IsNone() && !Row.PetBaseDataAsset.IsNull());

	UTexture2D* Texture = Row.PetIcon.LoadSynchronous();
	PetIcon->SetBrushFromTexture(Texture);
	PetIcon->SetVisibility(Texture ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	SetSelected(false);
}

FName ULobbyPetEntryWidget::GetPetRowName() const
{
	return PetRowName;
}

void ULobbyPetEntryWidget::SetSelected(bool bSelected)
{
	if (SelectionBorder)
	{
		SelectionBorder->SetVisibility(bSelected ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}
}

void ULobbyPetEntryWidget::HandlePetButtonClicked()
{
	if (!PetRowName.IsNone())
	{
		OnPetClicked.Broadcast(PetRowName);
	}
}
