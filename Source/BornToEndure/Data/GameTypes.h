/**
 * @file GameTypes.h
 * @brief 게임에서 사용되는 다양한 데이터 타입과 구조체를 정의하는 헤더
 * 
 * - 이 파일은 게임에서 사용되는 다양한 데이터 타입과 구조체를 정의
 * - 경험치, 골드, Pet 관련 데이터 구조체 포함
 * - Level Up 보상과 관련된 데이터 번들 정의 (PlayerController에서 LevelUp 시 Pet 목록과 Item 목록을 하나의 구조체로 담아 전달)
 */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Delegates/Delegate.h"
#include "Data/DataTableRow/ItemDataRow.h"

#include "GameTypes.generated.h"

class APetCompanionCharacter;
class UObject;

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

USTRUCT()
struct FLevelUpDataBundle
{
	GENERATED_BODY()

public:

	// 각 PetCharacter를 참조하는 TArray를 만들고, 해당 Gettter에서 이름 등을 가져오기?
	UPROPERTY()
	TArray<TObjectPtr<APetCompanionCharacter>> PetList;

	// Item에 대한 정보들 
	UPROPERTY()
	TArray<TObjectPtr<UObject>> RandomItemList;
};
