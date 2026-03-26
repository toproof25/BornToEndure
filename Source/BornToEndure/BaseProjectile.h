// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
<<<<<<< HEAD
#include "Poolable.h"
#include "BaseProjectile.generated.h"

// 로그 분류 커스텀
DECLARE_LOG_CATEGORY_EXTERN(LogBaseProjectile, Log, All);
=======
#include "BaseProjectile.generated.h"

>>>>>>> main

class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;
class UDamageType;
<<<<<<< HEAD
struct FTimerHandle;

UCLASS(Abstract, BlueprintType, Blueprintable)
class BORNTOENDURE_API ABaseProjectile : public AActor, public IPoolable
=======

UCLASS(BlueprintType, Blueprintable)
class BORNTOENDURE_API ABaseProjectile : public AActor
>>>>>>> main
{
	GENERATED_BODY()

public:
	ABaseProjectile();

protected:
	virtual void BeginPlay() override;

<<<<<<< HEAD
	virtual void ActivateActor_Implementation() override;
	virtual void DeactivateActor_Implementation() override;

=======
>>>>>>> main
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

<<<<<<< HEAD
	// 생명 관리
	FTimerHandle LifeSpanTimerHandle;
	FTimerDelegate TimerDelegate;

=======
>>>>>>> main
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile | Physics")
	//float PhysicsForce;

	UFUNCTION()
<<<<<<< HEAD
	virtual void OnProjectileHit(
=======
	void OnProjectileHit(
>>>>>>> main
		UPrimitiveComponent* HitComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

<<<<<<< HEAD
	//// 수명이 다했을 때 풀로 돌아가는 함수
	//UFUNCTION()
	//void ReturnToPool();
=======
>>>>>>> main

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile | Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComp;
<<<<<<< HEAD

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile | Components")
	TObjectPtr<USphereComponent> SphereComp;


public:

	void GetProjectileMovementComponent(UProjectileMovementComponent*& OutProjectileMovementComp) const { OutProjectileMovementComp = ProjectileMovementComp; }
	void GetSphereComponent(USphereComponent*& OutSphereComp) const { OutSphereComp = SphereComp; }
=======
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile | Components")
	TObjectPtr<USphereComponent> SphereComp;

public:

	UFUNCTION()
	void GetProjectileMovementComponent(UProjectileMovementComponent*& OutProjectileMovementComp) const { OutProjectileMovementComp = ProjectileMovementComp; }

>>>>>>> main

};
