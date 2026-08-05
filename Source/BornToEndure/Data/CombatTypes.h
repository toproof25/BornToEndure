/**
 * @file CombatTypes.h
 * @brief 전투 관련된 데이터 타입과 구조체를 정의하는 헤더
 */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Data/DataTableRow/ItemDataRow.h"
#include "Data/PetProjectileItemDataAsset.h"
#include "CombatTypes.generated.h"

class AActor;

 /**
  * @brief 공격 정보를 담는 구조체
  * @details
  * - 공격 데미지, 크리티컬 여부, 속성 태그, 발사체 구성 등 공격 실행에 필요한 모든 정보를 포함
  * - 매 공격 시 스탯과 아이템 정보를 조합하여 BuildAttackInfo()에서 생성되며, 최종적으로 발사체에 전달됨
  * - 생성된 후 버려지기에 스탯이 중간에 변경되어서 이미 공격된 공격은 영향을 받지 않음
  */
USTRUCT()
struct FPetAttackInfo
{
	GENERATED_BODY()

	// 최종 데미지, 크리티컬 여부, 크리티컬 배율
	float FinalDamage = 0.f;
	bool bIsCritical = false;
	float CriticalMultiplier = 1.5f;

	// 속성
	FGameplayTag ElementTag;
	float FireDamageBonus = 0.f;
	float IceDamageBonus = 0.f;
	float WindDamageBonus = 0.f;
	float PoisonDamageBonus = 0.f;
	float BleedDamageBonus = 0.f;

	// 발사체 구성
	TSubclassOf<AActor> ProjectileClass;
	int32 ProjectileCount = 1;
	float ProjectileSpeed = 1200.f;
	float ProjectileSize = 1.f;
	EProjectilePattern Pattern = EProjectilePattern::Single;
};
