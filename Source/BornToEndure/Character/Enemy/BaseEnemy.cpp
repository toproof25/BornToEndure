// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/BaseEnemy.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Subsystem/EffectSubsystem.h"
#include "NiagaraComponent.h"
#include "Delegates/Delegate.h"
#include "UObject/PrimaryAssetId.h"


DEFINE_LOG_CATEGORY(LogBaseEnemy);

ABaseEnemy::ABaseEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	EnemyMesh = CreateDefaultSubobject <UStaticMeshComponent>(TEXT("EnemyMesh"));
	EnemyColision = CreateDefaultSubobject <UCapsuleComponent>(TEXT("EnemyColision"));


	EnemyMesh->SetupAttachment(EnemyColision);

	EnemyColision->InitCapsuleSize(40.0f, 92.0f);


	RootComponent = EnemyColision;

}

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	UWorld* world = GetWorld();
	if (world == nullptr) return;
	UEffectSubsystem* EffectSubsystem = world->GetSubsystem<UEffectSubsystem>();
	check(EffectSubsystem);

	// 사용하는 Asset Preload
	FPrimaryAssetType SoundIdType(FName(TEXT("SoundDataAsset")));
	FPrimaryAssetType NiagaraType(FName(TEXT("NiagaraDataAsset")));


	FPrimaryAssetId HitSoundIdId(SoundIdType, HitEnemySoundId.PrimaryAssetName);
	FPrimaryAssetId HitNiagaraId(NiagaraType, HitEnemyNiagaraId.PrimaryAssetName);

	EffectSubsystem->PreloadEffectAssets(HitSoundIdId);
	EffectSubsystem->PreloadEffectAssets(HitNiagaraId);

	//OnEnemyHitSound.BindUObject(EffectSubsystem, &UEffectSubsystem::SpawnSoundAtLocation);
	//OnEnemyHitNiagara.BindUObject(EffectSubsystem, &UEffectSubsystem::SpawnNiagaraAtLocation);;
	
	CurrentHealth = MaxHealth;

	UE_LOG(LogBaseEnemy, Warning, TEXT("Spawn Test Enemy!!"));
}

void ABaseEnemy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UWorld* world = GetWorld();
	if (world == nullptr) return;
	UEffectSubsystem* EffectSubsystem = world->GetSubsystem<UEffectSubsystem>();
	check(EffectSubsystem);

	// 사용하는 Asset Unload
	FPrimaryAssetType SoundIdType(FName(TEXT("SoundDataAsset")));
	FPrimaryAssetType NiagaraType(FName(TEXT("NiagaraDataAsset")));

	FPrimaryAssetId HitSoundIdId(SoundIdType, HitEnemySoundId.PrimaryAssetName);
	FPrimaryAssetId HitNiagaraId(NiagaraType, HitEnemyNiagaraId.PrimaryAssetName);

	EffectSubsystem->UnloadEffectAssets(HitSoundIdId);
	EffectSubsystem->UnloadEffectAssets(HitNiagaraId);

	Super::EndPlay(EndPlayReason);
}


void ABaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

float ABaseEnemy::TakeDamage(
	float DamageAmount,
	struct FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser
)
{
	if (HitEnemySoundId.IsValid())
	{
		//OnEnemyHitSound.ExecuteIfBound(HitEnemySoundId.PrimaryAssetName, GetActorLocation());
	}
	if (HitEnemyNiagaraId.IsValid())
	{
		//OnEnemyHitNiagara.ExecuteIfBound(HitEnemyNiagaraId.PrimaryAssetName, GetActorLocation());
	}

	// 대충 물리 방어력 적용했다고 가정
	DamageAmount -= 5;
	CurrentHealth -= DamageAmount;
	if (CurrentHealth <= 0)
	{
		// Handle enemy death
		UE_LOG(LogBaseEnemy, Warning, TEXT("Enemy died"));
		Destroy();
	}


	FString CauserName = DamageCauser ? DamageCauser->GetName() : TEXT("Unknown");
	UE_LOG(LogBaseEnemy, Warning, TEXT("Enemy took damage: %f, Attacker: %s"), DamageAmount, *CauserName);

	return DamageAmount;
}
