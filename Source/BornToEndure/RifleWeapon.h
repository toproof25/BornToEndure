
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "RifleWeapon.generated.h"


class UArrowComponent;
class ABaseProjectile;

UCLASS()
class BORNTOENDURE_API ARifleWeapon : public AWeaponBase
{
	GENERATED_BODY()

public:
	ARifleWeapon();

protected:
	virtual void BeginPlay() override;
	virtual void Attack() override;

public:
	virtual void Tick(float DeltaTime) override;


public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UArrowComponent> ProjectilePoint;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<ABaseProjectile> ProjectileClass;


	virtual void InitializeProjectilePool() override;



};
