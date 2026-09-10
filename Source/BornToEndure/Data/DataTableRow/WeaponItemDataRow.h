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

class UPetWeaponItemDataAsset;

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	None UMETA(DisplayName = "None"),
	Fire UMETA(DisplayName = "Fire"),
	Melee UMETA(DisplayName = "Melee"),
	Beam UMETA(DisplayName = "Beam"),
};


USTRUCT(BlueprintType)
struct FWeaponItemDataRow : public FItemDataRow
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponItem Data", meta = (AllowedClasses = "PetWeaponItemDataAsset"))
	TSoftObjectPtr<UPetWeaponItemDataAsset> WeaponItemDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponItem Data")
	EWeaponType WeaponType;
};
