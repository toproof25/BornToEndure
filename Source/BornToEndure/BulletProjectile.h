// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseProjectile.h"
#include "BulletProjectile.generated.h"

/**
 * 
 */
UCLASS()
class BORNTOENDURE_API ABulletProjectile : public ABaseProjectile
{
	GENERATED_BODY()
	
public:
	ABulletProjectile();

protected:
	virtual void BeginPlay() override;


	virtual void OnProjectileHit(
		UPrimitiveComponent* HitComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	) override;

public:
	virtual void Tick(float DeltaTime) override;

};
