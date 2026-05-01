/**
 * @file PetItemProviderInterface.h
 * @brief Item 관련 기능에 대해 공통적으로 필요한 기능을 정의하는 인터페이스
 * @author toproof (kmnlmn123@gmail.com)
 * @date 2026-04-20
 * @details
 * - 외부에서 Pet의 Item에 접근하지 않아도 최종 계산된 발사체 modifier와 주요 속성 태그를 반환하는 기능을 제공한다.
 * - 결합도와 의존성을 줄이기 위해 Interface로 구현
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Data/PetProjectileItemDataAsset.h"
#include "GameplayTagContainer.h"
#include "PetItemProviderInterface.generated.h"


UINTERFACE(MinimalAPI, BlueprintType)
class UPetItemProviderInterface : public UInterface
{
	GENERATED_BODY()
};

class BORNTOENDURE_API IPetItemProviderInterface
{
	GENERATED_BODY()

public:
    /**
     * @brief 현재 장착된 모든 발사체 modifier를 합산한 결과를 반환한다.
     * - PetCombatComponent가 이 값으로 발사체를 구성한다.
     */
    virtual FProjectileModifierData GetAggregatedProjectileModifier() const = 0;

    /**
     * @brief 현재 Pet의 주 속성 태그를 반환한다.
     * - 불 아이템이 가장 많으면 "Item.Element.Fire" 반환
     */
    virtual FGameplayTag GetDominantElementTag() const = 0;
};