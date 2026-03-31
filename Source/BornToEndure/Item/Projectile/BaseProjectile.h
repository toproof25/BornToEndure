// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/Poolable.h"
#include "Delegates/Delegate.h"
#include "BaseProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;
class UDamageType;
class UNiagaraComponent;
class UAudioComponent;
struct FTimerHandle;

// 로그 분류 커스텀
DECLARE_LOG_CATEGORY_EXTERN(LogBaseProjectile, Log, All);

// Delegate 선언
DECLARE_DELEGATE_TwoParams(FOnProjectileHitSound, FName, FVector);
DECLARE_DELEGATE_TwoParams(FOnProjectileHitNiagara, FName, FVector);


UCLASS(Abstract, BlueprintType, Blueprintable)
class BORNTOENDURE_API ABaseProjectile : public AActor, public IPoolable
{
	GENERATED_BODY()

public:
	ABaseProjectile();

protected:
	virtual void BeginPlay() override;
	virtual void ActivateActor_Implementation() override;
	virtual void DeactivateActor_Implementation() override;

public:
	virtual void Tick(float DeltaTime) override;

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile | Mesh")
	TObjectPtr<UStaticMeshComponent> ProjectileMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile | Damage")
	float ProjectileDamage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile | Lifespan")
	float ProjectileLifespan;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile | Damage")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<AActor> TargetActor;

	// 생명 관리
	FTimerHandle LifeSpanTimerHandle;
	FTimerDelegate TimerDelegate;

	UFUNCTION()
	virtual void OnProjectileHit(
		UPrimitiveComponent* HitComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile | Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile | Components")
	TObjectPtr<USphereComponent> SphereComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile | Components")
	TObjectPtr<UNiagaraComponent> NiagaraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile | Components")
	TObjectPtr<UAudioComponent> AudioComp;

private:
	FOnProjectileHitSound OnProjectileHitSound;
	FOnProjectileHitNiagara OnProjectileHitNiagara;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile | Effects | Sound", meta = (AllowedTypes = "SoundDataAsset"))
	FPrimaryAssetId HitSoundId;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile | Effects | Niagara", meta = (AllowedTypes = "NiagaraDataAsset"))
	FPrimaryAssetId HitNiagaraId;

public:

	void GetProjectileMovementComponent(UProjectileMovementComponent*& OutProjectileMovementComp) const { OutProjectileMovementComp = ProjectileMovementComp; }
	void GetSphereComponent(USphereComponent*& OutSphereComp) const { OutSphereComp = SphereComp; }

};
