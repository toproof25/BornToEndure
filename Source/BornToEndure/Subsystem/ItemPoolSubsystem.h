/**
* @file ItemPoolSubsystem.h
* @date 2026-06-06
* @modified 2026-06-06
* @author toproof (kmnlmn123@gmail.com)
* @brief 아이템 데이터를 보관하고 조회와 무기 추첨을 제공합니다.
* - Level Up 시 랜덤 아이템을 제공하는 기능 구현에 활용
*/

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "Data/DataTableRow/PetDataRow.h"

#include "Data/DataTableRow/ItemDataRow.h"
#include "Data/DataTableRow/StatItemDataRow.h"
#include "Data/DataTableRow/WeaponItemDataRow.h"

#include "ItemPoolSubsystem.generated.h"

class UDataTable;

DECLARE_LOG_CATEGORY_EXTERN(LogItemPoolSubsystem, Log, All);


UCLASS()
class BORNTOENDURE_API UItemPoolSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UDataTable* PetDataTable;
	TArray<FPetDataRow*> PetDataList;


	UDataTable* StatDataTable;
	UDataTable* WeaponDataTable;

	TArray<FItemDataRow*> ItemDataList;
	TArray<FItemDataRow*> StatItemDataList;
	TArray<FItemDataRow*> WeaponItemDataList;

	void InitializeItemPoolSubsystem(UDataTable* ItemDataTable, int32 DataType);
	void InitializePetDataTable(UDataTable* InPetDataTable);

	/**
	 * 기존 호출자를 위해 현재 무기 추첨 API로 위임합니다.
	 * @param Count 요청할 최대 상품 수입니다.
	 * @return GetRandomWeaponItemData의 추첨 결과입니다.
	 */
	TArray<FItemDataHandle> GetRandomItemData(int32 Count);

	/**
	 * 무기 임시 목록에서 선택한 인덱스를 제거하며 추첨합니다.
	 * @param Count 요청할 최대 상품 수입니다.
	 * @return 요청 수 이하의 핸들. 풀이 비었거나 유효 행이 부족하면 더 적게 반환합니다.
	 * @note 같은 호출에서는 선택 인덱스를 재추첨하지 않으며, 다음 호출에는 이전 상품이 다시 등장할 수 있습니다.
	 */
	TArray<FItemDataHandle> GetRandomWeaponItemData(int32 Count);

	const FPetDataRow* GetPetDataRowByID(const FName& PetID);

	const FItemDataRow* GetItemDataRowByID(const EItemType ItemType, const FName& ItemID);
	const FStatItemDataRow* GetStatItemDataRowByID(const FName& ItemID);

	/**
	 * 아이템 ID에 대응하는 무기 행을 조회합니다.
	 * @param ItemID 무기 테이블의 행 이름입니다.
	 * @return 테이블 미초기화 또는 행 조회 실패 시 nullptr입니다.
	 */
	const FWeaponItemDataRow* GetWeaponItemDataRowByID(const FName& ItemID);
};
