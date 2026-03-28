
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "EffectSubsystem.h"
#include "Delegates/Delegate.h"
#include "RifleWeapon.generated.h"


class UArrowComponent;
class ABaseProjectile;

class USoundBase;
class UParticleSystem;

DECLARE_DELEGATE_TwoParams(FSpawnSoundAtLocation, FString, FVector);

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TObjectPtr<UParticleSystem> FireParticle;


	virtual void InitializeProjectilePool() override;

	FSpawnSoundAtLocation EffectSoundDelegate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|Sound", meta = (AllowedTypes = "SoundDataAsset"))
	FPrimaryAssetId AttackSoundId;
};
