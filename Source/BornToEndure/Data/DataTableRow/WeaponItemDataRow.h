/**
* @file WeaponItemDataRow.h
* @date 2026-08-26
* @brief Weapon Item 데이터 테이블 행 구조체를 정의하는 헤더 파일
*/
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Data/DataTableRow/ItemDataRow.h"
#include "WeaponItemDataRow.generated.h"

class UPetItemDataAsset;
class UTexture2D;


USTRUCT(BlueprintType)
struct FWeaponItemDataRow : public FItemDataRow
{
	GENERATED_BODY()

public:


};
