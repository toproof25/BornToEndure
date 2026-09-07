/**
* @file EnemyDataRow.h
* @date 2026-05-19
* @modified 2026-05-23
* @author toproof (kmnlmn123@gmail.com)
* @brief 모든 아이템을 테이블로 관리하기 위한 아이템 데이터 테이블 행 구조체 (데이터 타입)을 정의하는 파일
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemDataRow.generated.h"

class UPetItemDataAsset;
class UTexture2D;

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Stat UMETA(DisplayName = "스탯 아이템"),
	Weapon UMETA(DisplayName = "무기 아이템")
};

USTRUCT()
struct FItemDataHandle
{
	GENERATED_BODY()

	EItemType ItemType;
	FName ItemRowName;
};


USTRUCT(BlueprintType)
struct FItemText
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Text")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Text")
	FText Description;
};

USTRUCT(BlueprintType)
struct FItemDataRow : public FTableRowBase
{
	GENERATED_BODY()

public:

	// 아이템 ID, 이름, 설명, 아이콘, DataAsset 등..
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FItemText ItemText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	EItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	TSoftObjectPtr<UTexture2D> ItemIcon;
};
