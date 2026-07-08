/**
* @file ItemPoolSubsystem.h
* @date 2026-06-06
* @modified 2026-06-06
* @author toproof (kmnlmn123@gmail.com)
* @brief 모든 아이템을 캐싱하여 관리하며, 아이템 관련 기능을 제공하는 서브시스템
* - Level Up 시 랜덤 아이템을 제공하는 기능 구현에 활용
*/

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/DataTableRow/ItemDataRow.h"

#include "ItemPoolSubsystem.generated.h"

class UDataTable;

DECLARE_LOG_CATEGORY_EXTERN(LogItemPoolSubsystem, Log, All);

UCLASS()
class BORNTOENDURE_API UItemPoolSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	TArray<FItemDataRow*> ItemDataList;

	void InitializeItemPoolSubsystem(UDataTable* ItemDataTable);

	TArray<TObjectPtr<UObject>> GetRandomItemObjects(int32 Count);
};
