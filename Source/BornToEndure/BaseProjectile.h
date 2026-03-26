// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseProjectile.generated.h"


class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;
class UDamageType;

UCLASS(BlueprintType, Blueprintable)
class BORNTOENDURE_API ABaseProjectile : public AActor
{
	GENERATED_BODY()

public:
	ABaseProjectile();

protected:
	virtual void BeginPlay() override;

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

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile | Physics")
	//float PhysicsForce;

	UFUNCTION()
	void OnProjectileHit(
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

public:

	UFUNCTION()
	void GetProjectileMovementComponent(UProjectileMovementComponent*& OutProjectileMovementComp) const { OutProjectileMovementComp = ProjectileMovementComp; }


};
