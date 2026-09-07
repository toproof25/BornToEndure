
#include "Subsystem/ItemPoolSubsystem.h"
#include "Engine/DataTable.h"
#include "Data/DataTableRow/ItemDataRow.h"
#include "Data/DataTableRow/StatItemDataRow.h"
#include "Data/DataTableRow/WeaponItemDataRow.h"

DEFINE_LOG_CATEGORY(LogItemPoolSubsystem);

void UItemPoolSubsystem::InitializeItemPoolSubsystem(UDataTable* ItemDataTable, int32 DataType)
{
	switch(DataType)
	{
		case 0:
			// Stat 아이템
			ItemDataTable->GetAllRows<FItemDataRow>(
				TEXT("UItemPoolSubsystem InitializeItemPoolSubsystemDebug: Actor LoadAllPetItemsAsync"), 
				StatItemDataList
			);
			StatDataTable = ItemDataTable;
			break;
		case 1:
			// Weapon 아이템
			ItemDataTable->GetAllRows<FItemDataRow>(
				TEXT("UItemPoolSubsystem InitializeItemPoolSubsystemDebug: Actor LoadAllPetItemsAsync"), 
				WeaponItemDataList
			);
			WeaponDataTable = ItemDataTable;
			break;
		default:
			UE_LOG(LogItemPoolSubsystem, Error, TEXT("[UItemPoolSubsystem] InitializeItemPoolSubsystem: 잘못된 DataType 값 (%d) 전달됨"), DataType);
			break;
	}
}

TArray<FItemDataHandle> UItemPoolSubsystem::GetRandomItemData(int32 Count)
{
	TArray<FItemDataHandle> ResultObjects;

	// 아이템 데이터가 없으면 빈 배열 반환
	if (StatItemDataList.IsEmpty() && WeaponItemDataList.IsEmpty()) return ResultObjects;

	// Count 만큼 랜덤 추출
	TArray<FItemDataRow*> TempList = StatItemDataList;
	//TempList.Append(WeaponItemDataList);
	int32 ActualCount = FMath::Min(Count, TempList.Num());

	for (int32 i = 0; i < ActualCount; ++i)
	{
		// 랜덤 인덱스 추출
		int32 RandomIndex = FMath::RandRange(0, TempList.Num() - 1);
		FItemDataRow* SelectedRow = TempList[RandomIndex];

		// UObject 래퍼 생성
		FItemDataHandle NewItemHandle;
		if (SelectedRow)
		{
			NewItemHandle.ItemType = SelectedRow->ItemType;
			NewItemHandle.ItemRowName = SelectedRow->ItemID;
			ResultObjects.Add(NewItemHandle);
		}

		// 아이템 제거
		TempList.RemoveAtSwap(RandomIndex);
	}

	return ResultObjects;
}

const FItemDataRow* UItemPoolSubsystem::GetItemDataRowByID(const EItemType ItemType, const FName& ItemID)
{
	FItemDataRow* FoundRow = nullptr;
	switch (ItemType)
	{
		case EItemType::Stat:
		{
			const FStatItemDataRow* StatRow =
				StatDataTable->FindRow<FStatItemDataRow>(
					ItemID,
					TEXT("Apply Stat Item")
				);

			if (StatRow)
			{
				// StatRow->StatModifiers 사용 가능
				// StatRow->SynergyTags 사용 가능
				return StatRow;
			}
			break;
		}

		case EItemType::Weapon:
		{
			const FWeaponItemDataRow* WeaponRow =
				WeaponDataTable->FindRow<FWeaponItemDataRow>(
					ItemID,
					TEXT("Apply Weapon Item")
				);

			if (WeaponRow)
			{
				// Weapon 전용 데이터 접근
				return WeaponRow;
			}
			break;
		}
	}
	return nullptr;
}


const FStatItemDataRow* UItemPoolSubsystem::GetStatItemDataRowByID(const FName& ItemID)
{
	const FStatItemDataRow* StatRow =
		StatDataTable->FindRow<FStatItemDataRow>(
			ItemID,
			TEXT("Apply Stat Item")
		);

	if (StatRow)
	{
		// StatRow->StatModifiers 사용 가능
		// StatRow->SynergyTags 사용 가능
		return StatRow;
	}

	return nullptr;
}
const FWeaponItemDataRow* UItemPoolSubsystem::GetWeaponItemDataRowByID(const FName& ItemID)
{
	const FWeaponItemDataRow* WeaponRow =
		WeaponDataTable->FindRow<FWeaponItemDataRow>(
			ItemID,
			TEXT("Apply Weapon Item")
		);

	if (WeaponRow)
	{
		// Weapon 전용 데이터 접근
		return WeaponRow;
	}
	return nullptr;
}
