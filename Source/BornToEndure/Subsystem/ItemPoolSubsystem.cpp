/**
 * @file ItemPoolSubsystem.cpp
 * @brief Pet과 아이템 조회 및 회차 내 중복 없는 무기 추첨을 처리합니다.
 */


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

void UItemPoolSubsystem::InitializePetDataTable(UDataTable* InPetDataTable)
{
	PetDataList.Reset();
	PetDataTable = nullptr;
	if (!IsValid(InPetDataTable) || !InPetDataTable->GetRowStruct()
		|| !InPetDataTable->GetRowStruct()->IsChildOf(FPetDataRow::StaticStruct()))
	{
		UE_LOG(LogItemPoolSubsystem, Error, TEXT("[UItemPoolSubsystem] Pet 데이터 테이블 또는 행 타입이 올바르지 않습니다."));
		return;
	}

	PetDataTable = InPetDataTable;
	PetDataTable->GetAllRows<FPetDataRow>(TEXT("InitializePetDataTable"), PetDataList);
}

TArray<FName> UItemPoolSubsystem::GetPetRowNames() const
{
	TArray<FName> RowNames;
	if (IsValid(PetDataTable))
	{
		RowNames = PetDataTable->GetRowNames();
		RowNames.Sort(FNameLexicalLess());
	}

	return RowNames;
}

TArray<FItemDataHandle> UItemPoolSubsystem::GetRandomItemData(int32 Count)
{
	return GetRandomWeaponItemData(Count);
}

TArray<FItemDataHandle> UItemPoolSubsystem::GetRandomWeaponItemData(int32 Count)
{
	TArray<FItemDataHandle> ResultObjects;

	// 아이템 데이터가 없으면 빈 배열 반환
	if (StatItemDataList.IsEmpty() && WeaponItemDataList.IsEmpty())
	{
		return ResultObjects;
	}

	// Count 만큼 랜덤 추출
	TArray<FItemDataRow*> TempList = WeaponItemDataList;
	int32 ActualCount = FMath::Min(Count, TempList.Num());

	for (int32 i = 0; i < ActualCount; ++i)
	{
		// 랜덤 인덱스 추출
		int32 RandomIndex = FMath::RandRange(0, TempList.Num() - 1);
		FItemDataRow* SelectedRow = TempList[RandomIndex];

		// 선택한 행의 식별 정보를 핸들에 저장합니다.
		FItemDataHandle NewItemHandle;
		if (SelectedRow)
		{
			NewItemHandle.ItemType = SelectedRow->ItemType;
			NewItemHandle.ItemRowName = SelectedRow->ItemID;
			ResultObjects.Add(NewItemHandle);
		}

		// 같은 호출에서 선택 인덱스가 다시 추첨되지 않도록 임시 목록에서 제거합니다.
		TempList.RemoveAtSwap(RandomIndex);
	}

	return ResultObjects;
}

const FPetDataRow* UItemPoolSubsystem::GetPetDataRowByID(const FName& PetID)
{
	if (!IsValid(PetDataTable) || PetID.IsNone())
	{
		return nullptr;
	}

	return PetDataTable->FindRow<FPetDataRow>(PetID, TEXT("GetPetDataRowByID"), false);
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
	if (!WeaponDataTable)
	{
		return nullptr;
	}
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
