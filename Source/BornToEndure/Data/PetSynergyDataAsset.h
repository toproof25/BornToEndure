#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Stat/PetStatTypes.h"
#include "Data/PetProjectileItemDataAsset.h"
#include "PetSynergyDataAsset.generated.h"


/**
 * @brief 시너지 하나를 정의하는 DataAsset
 * - 시너지 이름, 발동 조건(태그 + 개수), 효과(스탯 증감, 공격 변화) 등을 포함
 * - 시너지의 발동 조건을 TMap<FGameplayTag, int32>로 표현하여 유연하게 여러 조건을 지원
 * - 코드를 수정하지 않고 에디터에서 새로운 시너지를 쉽게 추가할 수 있도록 설계
 * - PetItemComponent가 모든 SynergyData 에셋을 로드하여 조건을 검사
 */
UCLASS(BlueprintType)
class BORNTOENDURE_API UPetSynergyDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Synergy")
    FText SynergyName;

    /**
     * @brief 시너지 발동 조건에 대한 TMap
     * Key: GameplayTag (예: "Item.Element.Fire")
     * Value: 필요한 개수 (예: 3)
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Synergy|Condition")
    TMap<FGameplayTag, int32> RequiredTagCounts;

    /**
     * @brief 시너지 발동 시 적용되는 스탯 증감
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Synergy|Effect")
    TArray<FStatModifier> StatBonuses;

    /**
     * @brief 시너지 발동 시 변화되는 공격 수정자 데이터
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Synergy|Effect")
    FProjectileModifierData ProjectileBonus;

    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId(FPrimaryAssetType("PetSynergy"), GetFName());
    }
};
