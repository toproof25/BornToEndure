/**
 * @file PetItemDataAsset.h
 * @brief 모든 Item에 대한 DataAsset 기반 클래스 정의
 * @author toproof (kmnlmn123@gmail.com)
 * @date 2026-04-20
 * @details
 * - 모든 아이템 공통 데이터와 기능을 정의
 * - Stat 증감 아이템, 공격 변화 아이템은 해당 클래스를 상속받아 구현함
 * - 해당 클래스 자체로 인스턴스를 만들 수 없도록 Abstract 키워드 사용
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "PetItemDataAsset.generated.h"

class UPetItemComponent;

/**
 * @brief 모든 Pet 아이템 DataAsset의 기반 클래스
 * - 이 클래스는 직접 인스턴스를 만들 수 없으며, 반드시 서브클래스를 통해 사용해야 한다.
 * - 아이템이 Pet에 적용될 때와 제거될 때의 동작을 정의하는 순수 가상 함수를 포함한다.
 * - 아이템이 가지는 시너지 태그들을 포함하여, PetItemComponent가 시너지 조건을 체크하는 데 사용된다.
 */
UCLASS(Abstract, BlueprintType)
class BORNTOENDURE_API UPetItemDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Identity")
    FText ItemName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Identity")
    FText ItemDescription;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Identity")
    TSoftObjectPtr<UTexture2D> ItemIcon;

    /**
     * @brief 아이템이 가지는 시너지 태그들
	 * @note 이 태그들은 PetItemComponent가 시너지 조건을 체크하는 데 사용됨
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Synergy")
    FGameplayTagContainer SynergyTags;

    /**
     * @brief 아이템이 Pet에 추가될 때 호출
     */
    virtual void ApplyToComponent(UPetItemComponent* ItemComp) const PURE_VIRTUAL(UPetItemData::ApplyToComponent, );

    /**
     * @brief 아이템이 Pet에서 제거될 때 호출
     */
    virtual void RemoveFromComponent(UPetItemComponent* ItemComp, const FGuid& InstanceId) const PURE_VIRTUAL(UPetItemData::RemoveFromComponent, );

    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId(FPrimaryAssetType("PetItem"), GetFName());
    }

};
