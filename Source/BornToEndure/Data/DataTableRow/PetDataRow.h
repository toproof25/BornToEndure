/**
* @file PetDataRow.h
* @brief Pet 데이터 테이블의 행을 정의하는 클래스
* - Pet의 아이콘, 이름, 설명과 Pet DataAsset 정보를 포함
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PetDataRow.generated.h"

class UPetBaseDataAsset;
class UTexture2D;

USTRUCT(BlueprintType)
struct BORNTOENDURE_API FPetDataRow : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pet")
	FName RowID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pet")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pet")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pet")
	TSoftObjectPtr<UTexture2D> PetIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pet")
	TSoftObjectPtr<UPetBaseDataAsset> PetBaseDataAsset;

};
