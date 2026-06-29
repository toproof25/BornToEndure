
#include "Subsystem/ItemPoolSubsystem.h"
#include "Engine/DataTable.h"
#include "Data/DataTableRow/ItemDataRow.h"

DEFINE_LOG_CATEGORY(LogItemPoolSubsystem);

void UItemPoolSubsystem::InitializeItemPoolSubsystem(UDataTable* ItemDataTable)
{
	ItemDataTable->GetAllRows<FItemDataRow>(
		TEXT("UItemPoolSubsystem InitializeItemPoolSubsystemDebug: Actor LoadAllPetItemsAsync"), 
		ItemDataList
	);

	UE_LOG(LogItemPoolSubsystem, Warning, TEXT("[UItemPoolSubsystem] --- 캐싱된 아이템 상세 정보 (최대 10개 : 보유 %d개) ---"), ItemDataList.Num());

	int32 LogLimit = FMath::Min(10, ItemDataList.Num());

	for (int32 i = 0; i < LogLimit; ++i)
	{
		FItemDataRow* ItemInfo = ItemDataList[i];

		if (ItemInfo)
		{
			FString AssetPath = ItemInfo->ItemDataAsset.IsNull() ? TEXT("None") : ItemInfo->ItemDataAsset.ToString();

			UE_LOG(LogItemPoolSubsystem, Log, TEXT("[Item %d] ID: %s | Name: %s | Type: %d"),
				i + 1,
				*ItemInfo->ItemID.ToString(),
				*ItemInfo->ItemText.Name.ToString(),
				(int32)ItemInfo->ItemType // Enum은 기본적으로 int32로 캐스팅하여 출력
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

TArray<TObjectPtr<UObject>> UItemPoolSubsystem::GetRandomItemObjects(int32 Count)
{
	TArray<TObjectPtr<UObject>> ResultObjects;

	// 아이템 데이터가 없으면 빈 배열 반환
	if (ItemDataList.IsEmpty()) return ResultObjects;

	// Count 만큼 랜덤 추출
	TArray<FItemDataRow*> TempList = ItemDataList;
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
