#pragma once

#include "CoreMinimal.h"
#include "Data/PetItemDataAsset.h"
#include "Stat/PetStatTypes.h"
#include "PetStatItemDataAsset.generated.h"

class UPetItemComponent;

/**
 * @brief 스탯 증감 아이템의 DataAsset 클래스
 * - GameTypes.h의 FStatModifier 구조체를 사용하여 아이템이 적용하는 스탯 수정을 정의
 */
UCLASS(BlueprintType)
class BORNTOENDURE_API UPetStatItemDataAsset : public UPetItemDataAsset
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

    virtual void ApplyToComponent(UPetItemComponent* ItemComp) const override;
    virtual void RemoveFromComponent(UPetItemComponent* ItemComp, const FGuid& InstanceId) const override;

};
