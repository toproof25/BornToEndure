// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletProjectile.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"

#include "BaseEnemy.h"

DEFINE_LOG_CATEGORY(LogBulletProjectile);

ABulletProjectile::ABulletProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABulletProjectile::BeginPlay()
{
	Super::BeginPlay();
}

void ABulletProjectile::OnProjectileHit(
	UPrimitiveComponent* HitComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit
)
{

	UE_LOG(LogBulletProjectile, Display, TEXT("OnProjectileHit called. OtherActor: %s"), OtherActor ? *OtherActor->GetName() : TEXT("nullptr"));

	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
	{

		if (OtherActor->IsA(ABaseEnemy::StaticClass()))
		{
			UE_LOG(LogBaseProjectile, Display, TEXT("Target is ABaseEnemy or child."));
		}
		else
		{
			UE_LOG(LogBaseProjectile, Warning, TEXT("Target is NOT ABaseEnemy: %s"), *OtherActor->GetName());
		}

		const AController* InstigatorCtrl = GetInstigatorController();
		UE_LOG(LogBaseProjectile, Display, TEXT("InstigatorController: %s"), InstigatorCtrl ? *InstigatorCtrl->GetName() : TEXT("nullptr"));

		if (DamageType == nullptr)
		{
			UE_LOG(LogBaseProjectile, Warning, TEXT("DamageType is nullptr!"));
		}

		// OtherActor에 ProjectileDamage만큼 피해를 입히는 ApplyDamage 함수 호출
		UE_LOG(LogBaseProjectile, Display, TEXT("Bullet hit: %s"), *OtherActor->GetName());
		UGameplayStatics::ApplyDamage(
			OtherActor,
			ProjectileDamage,
			GetInstigatorController(),
			this,
			DamageType
		);


		// 부모 클래스에서 제거됨
		Super::OnProjectileHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
	}
}

void ABulletProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

