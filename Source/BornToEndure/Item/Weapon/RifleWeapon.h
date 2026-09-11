
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Weapon/BaseWeapon.h"
#include "RifleWeapon.generated.h"

UCLASS()
class BORNTOENDURE_API ARifleWeapon : public ABaseWeapon
{
	GENERATED_BODY()

public:
	ARifleWeapon();
protected:
	virtual void BeginPlay() override;

	virtual void OnAttack(const FPetAttackInfo& AtkInfo, const FVector& TargetLocation) override {};

};
