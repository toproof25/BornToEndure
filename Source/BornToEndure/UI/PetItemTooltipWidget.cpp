#include "UI/PetItemTooltipWidget.h"
#include "Subsystem/ItemPoolSubsystem.h"
#include "Components/TextBlock.h"
#include "Data/DataTableRow/ItemDataRow.h"
#include "Data/DataTableRow/StatItemDataRow.h"
#include "Data/DataTableRow/WeaponItemDataRow.h"

void UPetItemTooltipWidget::UpdateTooltip(const FItemDataHandle& ItemDataHandle)
{
	UWorld* World = GetWorld();
	if (!World) return;
	UItemPoolSubsystem* ItemPoolSubsystem = World->GetGameInstance()->GetSubsystem<UItemPoolSubsystem>();
	if (!ItemPoolSubsystem) return;

	if (ItemDataHandle.ItemType == EItemType::Stat)
	{
		const FStatItemDataRow* StatItemDataRow = ItemPoolSubsystem->GetStatItemDataRowByID(ItemDataHandle.ItemRowName);
		if (StatItemDataRow)
		{
			const FItemText& ItemText = StatItemDataRow->ItemText;
			ItemNameText->SetText(ItemText.Name);
			ItemDescriptionText->SetText(ItemText.Description);
		}
	}
	else if (ItemDataHandle.ItemType == EItemType::Weapon)
	{
		const FWeaponItemDataRow* WeaponItemDataRow = ItemPoolSubsystem->GetWeaponItemDataRowByID(ItemDataHandle.ItemRowName);
		if (WeaponItemDataRow)
		{
			const FItemText& ItemText = WeaponItemDataRow->ItemText;
			ItemNameText->SetText(ItemText.Name);
			ItemDescriptionText->SetText(ItemText.Description);
		}
	}
}
