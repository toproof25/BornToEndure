
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
			break;
		case 1:
			// Weapon 아이템
			ItemDataTable->GetAllRows<FItemDataRow>(
				TEXT("UItemPoolSubsystem InitializeItemPoolSubsystemDebug: Actor LoadAllPetItemsAsync"), 
				WeaponItemDataList
			);
			break;
		default:
			UE_LOG(LogItemPoolSubsystem, Error, TEXT("[UItemPoolSubsystem] InitializeItemPoolSubsystem: 잘못된 DataType 값 (%d) 전달됨"), DataType);
			break;
	}

	if (DataType == 0)
	{
		UE_LOG(LogItemPoolSubsystem, Warning, TEXT("[UItemPoolSubsystem] --- 캐싱된 아이템 상세 정보 (최대 10개 : 보유 %d개) DataType - %d ---"), StatItemDataList.Num(), DataType);

		int32 LogLimit = FMath::Min(10, StatItemDataList.Num());

		for (int32 i = 0; i < LogLimit; ++i)
		{
			FItemDataRow* ItemInfo = StatItemDataList[i];

			if (ItemInfo)
			{
				FString AssetPath = ItemInfo->ItemDataAsset.IsNull() ? TEXT("None") : ItemInfo->ItemDataAsset.ToString();

				UE_LOG(LogItemPoolSubsystem, Log, TEXT("[Item %d] ID: %s | Name: %s"),
					i + 1,
					*ItemInfo->ItemID.ToString(),
					*ItemInfo->ItemText.Name.ToString()
				);

				// 설명과 에셋 경로는 길어질 수 있으므로 다음 줄에 들여쓰기로 출력
				UE_LOG(LogItemPoolSubsystem, Log, TEXT("   ㄴ Description: %s"), *ItemInfo->ItemText.Description.ToString());
				UE_LOG(LogItemPoolSubsystem, Log, TEXT("   ㄴ DataAsset  : %s"), *AssetPath);
			}
			else
			{
				UE_LOG(LogItemPoolSubsystem, Warning, TEXT("[UItemPoolSubsystem] ItemDataRow 포인터가 null입니다. 인덱스: %d"), i);
			}
		}

		UE_LOG(LogItemPoolSubsystem, Warning, TEXT("[UItemPoolSubsystem] --- 아이템 상세 정보 출력 완료 ---"));
	}
	else
	{
		UE_LOG(LogItemPoolSubsystem, Warning, TEXT("[UItemPoolSubsystem] --- 캐싱된 아이템 상세 정보 (최대 10개 : 보유 %d개) DataType - %d ---"), WeaponItemDataList.Num(), DataType);

		int32 LogLimit = FMath::Min(10, WeaponItemDataList.Num());

		for (int32 i = 0; i < LogLimit; ++i)
		{
			FItemDataRow* ItemInfo = WeaponItemDataList[i];

			if (ItemInfo)
			{
				FString AssetPath = ItemInfo->ItemDataAsset.IsNull() ? TEXT("None") : ItemInfo->ItemDataAsset.ToString();
				UE_LOG(LogItemPoolSubsystem, Log, TEXT("[Item %d] ID: %s | Name: %s"),
					i + 1,
					*ItemInfo->ItemID.ToString(),
					*ItemInfo->ItemText.Name.ToString()
				);

				// 설명과 에셋 경로는 길어질 수 있으므로 다음 줄에 들여쓰기로 출력
				UE_LOG(LogItemPoolSubsystem, Log, TEXT("   ㄴ Description: %s"), *ItemInfo->ItemText.Description.ToString());
				UE_LOG(LogItemPoolSubsystem, Log, TEXT("   ㄴ DataAsset  : %s"), *AssetPath);
			}
			else
			{
				UE_LOG(LogItemPoolSubsystem, Warning, TEXT("[UItemPoolSubsystem] ItemDataRow 포인터가 null입니다. 인덱스: %d"), i);
			}
		}

		UE_LOG(LogItemPoolSubsystem, Warning, TEXT("[UItemPoolSubsystem] --- 아이템 상세 정보 출력 완료 ---"));

	}


}

TArray<TObjectPtr<UObject>> UItemPoolSubsystem::GetRandomItemObjects(int32 Count)
{
	TArray<TObjectPtr<UObject>> ResultObjects;

	// 아이템 데이터가 없으면 빈 배열 반환
	if (StatItemDataList.IsEmpty() && WeaponItemDataList.IsEmpty()) return ResultObjects;

	// Count 만큼 랜덤 추출
	TArray<FItemDataRow*> TempList = StatItemDataList;
	TempList.Append(WeaponItemDataList);
	int32 ActualCount = FMath::Min(Count, TempList.Num());

	for (int32 i = 0; i < ActualCount; ++i)
	{
		// 랜덤 인덱스 추출
		int32 RandomIndex = FMath::RandRange(0, TempList.Num() - 1);
		FItemDataRow* SelectedRow = TempList[RandomIndex];

		// UObject 래퍼 생성
		UItemDataObject* NewItemObj = NewObject<UItemDataObject>();
		if (NewItemObj && SelectedRow)
		{
			NewItemObj->ItemData = *SelectedRow; // 구조체 데이터 복사
			ResultObjects.Add(NewItemObj);
		}

		// 아이템 제거
		TempList.RemoveAtSwap(RandomIndex);
	}

	return ResultObjects;
}

TSoftObjectPtr<UPetItemDataAsset> UItemPoolSubsystem::GetItemDataAssetByID(const FName& ItemID)
{
	// 아이템 데이터가 없으면 빈 배열 반환
	if (StatItemDataList.IsEmpty() && WeaponItemDataList.IsEmpty()) return TSoftObjectPtr<UPetItemDataAsset>();

	// Count 만큼 랜덤 추출
	TArray<FItemDataRow*> TempList = StatItemDataList;
	TempList.Append(WeaponItemDataList);

	for (auto ItemInfo : TempList)
	{
		if (ItemInfo && ItemInfo->ItemID == ItemID)
		{
			return ItemInfo->ItemDataAsset;
		}
	}

	return TSoftObjectPtr<UPetItemDataAsset>();
}
