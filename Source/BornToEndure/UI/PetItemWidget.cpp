#include "UI/PetItemWidget.h"

#include "Subsystem/ItemPoolSubsystem.h"

#include "UI/PetItemSlotWidget.h"
#include "UI/PetItemTooltipWidget.h"

#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"

#include "Data/DataTableRow/ItemDataRow.h"
#include "Data/DataTableRow/StatItemDataRow.h"
#include "Data/DataTableRow/WeaponItemDataRow.h"


void UPetItemWidget::UpdateOwnedItems(TArray<FItemDataHandle> InOwnedItemRowHandles)
{

	if (!ItemGrid || !PetItemSlotWidgetClass) return;

	ItemGrid->ClearChildren();
	constexpr int32 ColumnCount = 4;
	int32 Index = 0;

	for (const FItemDataHandle& ItemDataHandle : InOwnedItemRowHandles)
	{
		if (ItemDataHandle.ItemType == EItemType::Weapon)
		{
			PetWeaponSlotWidget->SetItemData(ItemDataHandle);
			continue;
		}

		UPetItemSlotWidget* ItemSlot = CreateWidget<UPetItemSlotWidget>(GetOwningPlayer(), PetItemSlotWidgetClass);
		if (!ItemSlot) continue;

		ItemSlot->SetItemData(ItemDataHandle);
		ItemSlot->OnItemSlotHovered.AddDynamic(this, &UPetItemWidget::HandleItemSlotHovered);

		const int32 Row = Index / ColumnCount;
		const int32 Column = Index % ColumnCount;

		ItemGrid->AddChildToUniformGrid(
			ItemSlot,
			Row,
			Column
		);

		++Index;
	}
}

void UPetItemWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (PetWeaponSlotWidget)
	{
		PetWeaponSlotWidget->OnItemSlotHovered.AddDynamic(this, &UPetItemWidget::HandleItemSlotHovered);
	}
}

void UPetItemWidget::NativeDestruct()
{
	if (PetWeaponSlotWidget)
	{
		PetWeaponSlotWidget->OnItemSlotHovered.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UPetItemWidget::HandleItemSlotHovered(const FItemDataHandle& ItemDataHandle, bool bIsHovered)
{
	PetItemTooltipWidget->UpdateTooltip(ItemDataHandle);
	UE_LOG(LogTemp, Log, TEXT("[PetItemWidget] HandleItemSlotHovered: ItemRowName: %s, bIsHovered: %s"), *ItemDataHandle.ItemRowName.ToString(), bIsHovered ? TEXT("true") : TEXT("false"));
}
