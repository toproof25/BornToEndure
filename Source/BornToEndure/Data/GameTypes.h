/**
 * @file GameTypes.h
 * @brief 게임에서 사용되는 다양한 데이터 타입과 구조체를 정의하는 헤더
 * @author toproof (kmnlmn123@gmail.com)
 * @date 2026-04-27
 * @details
 * - 이 파일은 게임에서 사용되는 다양한 데이터 타입과 구조체를 정의
 * - 경험치, 골드, Pet 관련 데이터 구조체 포함
 */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Delegates/Delegate.h"

#include "GameTypes.generated.h"


typedef TMap<FName, float> FPetDamageMap;

USTRUCT(BlueprintType)
struct BORNTOENDURE_API FEnemyRewardPayload
{
	GENERATED_BODY()

	float ExpReward = 0.f;
	int32 GoldReward = 0;

	// Pet 정보 취합할 때 사용
	FName KillerPetId = NAME_None;
	FPetDamageMap PetDamageMap;
	float TotalDamageReceiced = 0.f;

	//FGameplayTagContainer KillTags;

	void Reset()
	{
		ExpReward = 0.f;
		GoldReward = 0;
		KillerPetId = NAME_None;
		PetDamageMap.Empty();
		TotalDamageReceiced = 0.f;
	}

	void RegisterDamage(FName DamageCauser, float DamageAmount)
	{
		if (DamageCauser != NAME_None)
		{
			PetDamageMap.FindOrAdd(DamageCauser) += DamageAmount;
			TotalDamageReceiced += DamageAmount;
		}
	}
};

// 적 사망 시 Player에게 Payload를 전달하는 Delegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyKilled, const FEnemyRewardPayload&, EnemyRewardPayload);