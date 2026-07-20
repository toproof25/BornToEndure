/**
 * @file PetBaseDataAsset.h
 * @brief Pet의 기본 데이터 에셋 헤더
 * @author toproof (kmnlmn123@gmail.com)
 * @date 2026-04-20
 * @details
 * - 이 파일은 Pet들의 기본 데이터를 정의
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Stat/PetStatTypes.h"
#include "PetBaseDataAsset.generated.h"

class USkeletalMesh;
class UTexture2D;
class UBehaviorTree;
class UPetProjectileItemDataAsset;


/**
 * @brief Pet의 기본 데이터 에셋 클래스
 * - Pet의 이름, 아이콘, 메시, 스탯, AI 행동 트리 등을 정의
 */
UCLASS(BlueprintType)
class BORNTOENDURE_API UPetBaseDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
    FText PetName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
    TSoftObjectPtr<UTexture2D> Icon;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
    TSoftObjectPtr<USkeletalMesh> PetMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
    FPetBaseStatSheet BaseStats;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    TSoftObjectPtr<UBehaviorTree> BehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (AllowClasses = "PetProjectileItemDataAsset"))
    TSoftObjectPtr<UPetProjectileItemDataAsset> DefaultProjectileClass;

    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId(FPrimaryAssetType("PetData"), GetFName());
    }

	FText GetPetName() const { return PetName; }
};
