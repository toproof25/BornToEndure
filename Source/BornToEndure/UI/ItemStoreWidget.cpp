/**
 * @file ItemStoreWidget.cpp
 * @brief 상점 목록과 골드 변경 알림을 화면에 반영합니다.
 */

#include "UI/ItemStoreWidget.h"
#include "UI/PetEntryWidget.h"
#include "Character/Pet/PetCompanionCharacter.h"
#include "UI/ItemStoreEntryWidget.h"
#include "UI/PetItemTooltipWidget.h"
#include "Core/DefaultPlayerController.h"
#include "Component/PlayerExperienceComponent.h"
#include "Components/HorizontalBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UItemStoreWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (TitleText)
	{
		TitleText->SetText(FText::FromString(TEXT("상점")));
	}
	if (RerollButton)
	{
		RerollButton->OnClicked.AddDynamic(this, &UItemStoreWidget::HandleReroll);
	}
	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UItemStoreWidget::HandleClose);
	}
}

void UItemStoreWidget::BindCurrency(UPlayerExperienceComponent* Component)
{
	if (CurrencyComponent)
	{
		CurrencyComponent->OnGoldChanged.RemoveDynamic(this, &UItemStoreWidget::HandleGoldChanged);
	}

	CurrencyComponent = Component;

	if (CurrencyComponent)
	{
		CurrencyComponent->OnGoldChanged.AddUniqueDynamic(this, &UItemStoreWidget::HandleGoldChanged);
		HandleGoldChanged(CurrencyComponent->GetCurrentGold());
	}
}

void UItemStoreWidget::ClearEntries()
{
	SelectedItemIndex = INDEX_NONE;
	for (UItemStoreEntryWidget* Entry : Entries)
	{
		if (!Entry)
		{
			continue;
		}

		Entry->SetSelectedVisual(false);
		Entry->OnPurchaseRequested.RemoveAll(this);
		Entry->OnItemHovered.RemoveAll(this);
	}

	Entries.Empty();
	if (ItemHorizontalBox)
	{
		ItemHorizontalBox->ClearChildren();
	}
	if (ItemTooltipWidget)
	{
		ItemTooltipWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

bool UItemStoreWidget::DisplayOffers(const TArray<FItemDataHandle>& Offers, const TArray<int32>& Prices)
{
	ClearEntries();

	if (!ItemHorizontalBox || !EntryWidgetClass || Offers.Num() != Prices.Num())
	{
		return false;
	}

	for (int32 Index = 0; Index < Offers.Num(); ++Index)
	{
		UItemStoreEntryWidget* Entry = CreateWidget<UItemStoreEntryWidget>(GetOwningPlayer(), EntryWidgetClass);
		if (!Entry)
		{
			ClearEntries();
			return false;
		}

		Entry->InitializeEntry(Index, Offers[Index], Prices[Index]);
		Entry->OnPurchaseRequested.AddUObject(this, &UItemStoreWidget::HandlePurchase);
		Entry->OnItemHovered.AddUObject(this, &UItemStoreWidget::HandleHover);
		ItemHorizontalBox->AddChildToHorizontalBox(Entry);
		Entries.Add(Entry);
	}

	if (CurrencyComponent)
	{
		HandleGoldChanged(CurrencyComponent->GetCurrentGold());
	}

	return true;
}

void UItemStoreWidget::HandleGoldChanged(int32 Gold)
{
	if (GoldText)
	{
		GoldText->SetText(FText::AsNumber(Gold));
	}

	for (UItemStoreEntryWidget* Entry : Entries)
	{
		if (Entry)
		{
			Entry->UpdateAvailability(Gold);
		}
	}
}

void UItemStoreWidget::MarkSold(int32 Index)
{
	if (Entries.IsValidIndex(Index) && Entries[Index])
	{
		Entries[Index]->MarkSold();
	}
}

void UItemStoreWidget::HandlePurchase(int32 Index)
{
	if (auto* PC = GetOwningPlayer<ADefaultPlayerController>())
	{
		PC->PurchaseStoreItem(Index);
	}
}

void UItemStoreWidget::HandleReroll()
{
	if (auto* PC = GetOwningPlayer<ADefaultPlayerController>())
	{
		PC->RerollStoreItems();
	}
}

void UItemStoreWidget::HandleClose()
{
	if (auto* PC = GetOwningPlayer<ADefaultPlayerController>())
	{
		PC->CloseItemStore();
	}
}

void UItemStoreWidget::HandleHover(int32 Index, const FItemDataHandle& Handle, bool bHovered)
{
	if (!bHovered || !Entries.IsValidIndex(Index) || !Entries[Index])
	{
		return;
	}

	if (Entries.IsValidIndex(SelectedItemIndex) && Entries[SelectedItemIndex])
	{
		Entries[SelectedItemIndex]->SetSelectedVisual(false);
	}

	SelectedItemIndex = Index;
	Entries[Index]->SetSelectedVisual(true);
	if (ItemTooltipWidget)
	{
		ItemTooltipWidget->UpdateTooltip(Handle);
		ItemTooltipWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UItemStoreWidget::Shutdown()
{
	BindCurrency(nullptr);
	ClearEntries();
	ClearPets();
}

void UItemStoreWidget::NativeDestruct()
{
	Shutdown();
	Super::NativeDestruct();
}

bool UItemStoreWidget::DisplayPets(const TArray<TObjectPtr<APetCompanionCharacter>>& Pets)
{
	ClearPets();
	if (!PetListBox || !PetEntryWidgetClass)
	{
		return false;
	}

	for (APetCompanionCharacter* Pet : Pets)
	{
		if (!IsValid(Pet))
		{
			continue;
		}

		UPetEntryWidget* Entry = CreateWidget<UPetEntryWidget>(GetOwningPlayer(), PetEntryWidgetClass);
		if (!Entry || !Entry->PetNameText || !Entry->PetIconImage || !Entry->PetSelectButton || !Entry->PetBoxBorder)
		{
			ClearPets();
			return false;
		}

		Entry->InitializeWithPetData(nullptr, Pet);
		Entry->OnPetSelectedDelegate.AddUniqueDynamic(this, &UItemStoreWidget::HandlePetSelected);
		PetListBox->AddChildToHorizontalBox(Entry);
		PetEntries.Add(Entry);
	}

	return true;
}

void UItemStoreWidget::ClearPets()
{
	for (UPetEntryWidget* Entry : PetEntries)
	{
		if (Entry)
		{
			Entry->OnPetSelectedDelegate.RemoveAll(this);
		}
	}

	PetEntries.Empty();
	if (PetListBox)
	{
		PetListBox->ClearChildren();
	}
}

void UItemStoreWidget::HandlePetSelected(APetCompanionCharacter* Pet)
{
	ADefaultPlayerController* PC = GetOwningPlayer<ADefaultPlayerController>();
	if (!PC || !PC->SelectStorePet(Pet))
	{
		return;
	}

	for (UPetEntryWidget* Entry : PetEntries)
	{
		if (Entry)
		{
			Entry->SetSelectedVisual(Entry->SelectedPet == Pet);
		}
	}
}
