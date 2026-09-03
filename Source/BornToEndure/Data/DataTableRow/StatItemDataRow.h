/**
* @file StatItemDataRow.h
* @date 2026-08-26
* @brief Stat Item 데이터 테이블 행 구조체를 정의하는 헤더 파일
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Data/DataTableRow/ItemDataRow.h"
#include "Stat/PetStatTypes.h"
#include "GameplayTagContainer.h"
#include "StatItemDataRow.generated.h"

class UPetItemDataAsset;
class UTexture2D;

USTRUCT(BlueprintType)
struct FStatItemDataRow : public FItemDataRow
{
	GENERATED_BODY()

public:

	/**
	 * @brief 1개 요소당 하나의 스탯 수정자를 가지게 된다
	 * @details
	 * - TArray<FStatModifier>로 여러 스탯을 동시에 수정
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Stat")
	TArray<FStatModifier> StatModifiers;

	/**
	 * @brief 아이템이 가지는 시너지 태그들
	 * @note 이 태그들은 PetItemComponent가 시너지 조건을 체크하는 데 사용됨
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Synergy")
	FGameplayTagContainer SynergyTags;

};
