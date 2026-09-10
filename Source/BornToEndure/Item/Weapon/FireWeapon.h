/**
* @file FireWeapon.h
* @brief 발사체를 가지는 무기 클래스
* - BaseProjectile를 소유하여 발사체를 통해 공격을 수행하는 무기 클래스
*/
#pragma once

#include "CoreMinimal.h"
#include "Item/Weapon/BaseWeapon.h"

#include "Data/DataTableRow/WeaponItemDataRow.h"
#include "Data/CombatTypes.h"

#include "FireWeapon.generated.h"

class UPetWeaponItemDataAsset;

UCLASS()
class BORNTOENDURE_API AFireWeapon : public ABaseWeapon
{
	GENERATED_BODY()
	
public:
	virtual void OnAttack(const FPetAttackInfo& AtkInfo) override;

protected:
	virtual void BeginPlay() override;
	virtual void OnInitalizeWeapon(const UPetWeaponItemDataAsset& ItemData) override;

private:


};
