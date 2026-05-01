/**
 * @file PetStatProviderInterface.h
 * @brief Stat 변화 아이템에 대해 공통적으로 필요한 기능을 정의하는 인터페이스
 * @author toproof (kmnlmn123@gmail.com)
 * @date 2026-04-20
 * @details
 * - Pet의 Stat이나 Item에 접근하지 않아도 최종 계산된 Stat 값을 반환하는 기능을 제공한다.
 * - 결합도와 의존성을 줄이기 위해 Interface로 구현
 */


#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Stat/PetStatTypes.h"
#include "PetStatProviderInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UPetStatProviderInterface : public UInterface
{
	GENERATED_BODY()
};


class BORNTOENDURE_API IPetStatProviderInterface
{
	GENERATED_BODY()

public:
    /**
     * @brief 최종 계산된 스탯 값을 반환한다.
     * - (기본값 + 아이템 보너스 + 시너지 보너스 모두 적용)
     * - 현재 PetStatComponent에서 오버라이드 하며 기능 구현
     */
    virtual float GetFinalStat(EPetStatType StatType) const = 0;

};
