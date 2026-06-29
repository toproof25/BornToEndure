#include "UI/LevelUpRewardWidget.h"
#include "Components/Button.h"
#include "Core/DefaultPlayerController.h"
#include "UI/PetEntryWidget.h"
#include "UI/ItemEntryWidget.h"
#include "Components/HorizontalBox.h"
#include "Character/Pet/PetCompanionCharacter.h"
#include "Component/PetManagerComponent.h"
#include "Data/DataTableRow/ItemDataRow.h"
#include "Data/PetItemDataAsset.h"

void ULevelUpRewardWidget::NativeOnInitialized()
{
	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &ULevelUpRewardWidget::ExitButtonClicked);
	}
}

void ULevelUpRewardWidget::ExitButtonClicked()
{
	if (!SelectedPet || !SelectedItem) return;

    ADefaultPlayerController* PC = Cast<ADefaultPlayerController>(GetOwningPlayer());
	if (!PC) return;

    FInputModeGameOnly InputMode;
    PC->SetInputMode(InputMode);
    PC->bShowMouseCursor = false;

	UPetManagerComponent* PetManager = PC->GetPawn()->FindComponentByClass<UPetManagerComponent>();
	if (!PetManager) return;	

	UItemDataObject* ItemDataObject = Cast<UItemDataObject>(SelectedItem);
	FItemDataRow* ItemData = ItemDataObject ? &(ItemDataObject->ItemData) : nullptr;
	if (!ItemData) return;
	PetManager->GiveItemToPet(SelectedPet, ItemData->ItemDataAsset.LoadSynchronous());
	UE_LOG(LogTemp, Warning, TEXT("[ULevelUpRewardWidget] ExitButtonClicked: Given item %s to pet %s"), *ItemData->ItemText.Name.ToString(), *SelectedPet->GetName());

	RemoveFromParent();
}
void ULevelUpRewardWidget::InitializeWithLevelUpData(const FLevelUpDataBundle& InLevelUpData)
{
	if (!PetListBox || !ItemListBox) return;

	// WBP에서 임시 UI들을 제거
	PetListBox->ClearChildren();
	ItemListBox->ClearChildren();

	//DisplayedPets = InLevelUpData.PetList;
	//DisplayedItems = InLevelUpData.RandomItemList;

	if (InLevelUpData.PetList.Num() <= 0 || !PetEntryWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ULevelUpRewardWidget] InitializeWithLevelUpData: No pets to display or PetEntryWidgetClass is not set."));
		return;
	}

	for (TObjectPtr<APetCompanionCharacter> Pet : InLevelUpData.PetList)
	{
		// Pet 위젯 초기화 로직 추가
		UPetEntryWidget* NewPetEntry = CreateWidget<UPetEntryWidget>(this, PetEntryWidgetClass);
		NewPetEntry->InitializeWithPetData(Pet);
		NewPetEntry->OnPetSelectedDelegate.AddDynamic(this, &ULevelUpRewardWidget::HandlePetSelected);
		PetListBox->AddChild(NewPetEntry);
	}


	if (InLevelUpData.RandomItemList.Num() <= 0 || !ItemEntryWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ULevelUpRewardWidget] InitializeWithLevelUpData: No items to display or ItemEntryWidgetClass is not set."));
		return;
	}

	for (TObjectPtr<UObject> Item : InLevelUpData.RandomItemList)
	{
		// Item 위젯 초기화 로직 추가
		UItemEntryWidget* NewItemEntry = CreateWidget<UItemEntryWidget>(this, ItemEntryWidgetClass);
		NewItemEntry->InitializeWithItemData(this, Item);
		NewItemEntry->OnItemSelectedDelegate.AddDynamic(this, &ULevelUpRewardWidget::HandleItemSelected);
		ItemListBox->AddChild(NewItemEntry);
	}


	UE_LOG(LogTemp, Log, TEXT("LevelUpRewardWidget initialized with PetList Num: %d, ItemList Num: %d"), DisplayedPets.Num(), DisplayedItems.Num());
}

void ULevelUpRewardWidget::HandlePetSelected(APetCompanionCharacter* InSelectedPet)
{
	SelectedPet = InSelectedPet;
	UE_LOG(LogTemp, Log, TEXT("Pet selected"));
}

void ULevelUpRewardWidget::HandleItemSelected(UObject* InSelectedItem)
{
	SelectedItem = InSelectedItem;
	UE_LOG(LogTemp, Log, TEXT("Item selected"));
}

void ULevelUpRewardWidget::OnSelectedItemWidget(UItemEntryWidget* InSelectedItemWidget)
{
	if (!InSelectedItemWidget) return;

	if (SelectedItemWidget)
	{
		SelectedItemWidget->SetSelectedVisual(false);
	}

	SelectedItemWidget = InSelectedItemWidget;
	SelectedItemWidget->SetSelectedVisual(true);
}


