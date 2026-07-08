// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Weapon/BaseWeapon.h"
#include "Character/Player/PlayerCharacter.h"

#include "Subsystem/EffectSubsystem.h"
#include "Delegates/Delegate.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogBaseWeapon);

ABaseWeapon::ABaseWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	WeaponStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponStaticMesh"));
	RootComponent = WeaponStaticMesh;

	// 물리 충돌 허용
	WeaponStaticMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponStaticMesh->SetSimulatePhysics(true);
	WeaponStaticMesh->SetCollisionProfileName(TEXT("BlockAll"));
}

void ABaseWeapon::Interact_Implementation(APlayerCharacter* InstigatorCharacter)
{
	if (InstigatorCharacter == nullptr) return;

	// 무기 메쉬의 물리와 콜리젼 제거
	WeaponStaticMesh->SetSimulatePhysics(false);
	WeaponStaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 플레이어 메쉬와 소켓에 무기 부착
	USkeletalMeshComponent* PlayerMesh = InstigatorCharacter->GetMesh();
	if (PlayerMesh)
	{
		AttachToComponent(PlayerMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("WeaponSocket"));
	}

	SetInstigator(InstigatorCharacter);
	InstigatorCharacter->SetWeaponBase(this);
	InitializeProjectilePool();

	UE_LOG(LogBaseWeapon, Warning, TEXT("Weapon Interacted: %s, Instigator set"), *InstigatorCharacter->GetName());
}

void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	check(World);
	UEffectSubsystem* EffectSubsystem = GetWorld()->GetSubsystem<UEffectSubsystem>();
	check(EffectSubsystem);

	SoundDelegate.BindUObject(EffectSubsystem, &UEffectSubsystem::SpawnSoundAtLocation);
	NiagaraDelegate.BindUObject(EffectSubsystem, &UEffectSubsystem::SpawnNiagaraAtLocation);
}


void ABaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void ABaseWeapon::OnAttackSound(const FVector& SpawnLocation) const
{
	if (AttackSoundId.IsValid())
	{
		SoundDelegate.ExecuteIfBound(AttackSoundId.PrimaryAssetName, SpawnLocation);
	}
}

void ABaseWeapon::OnAttackNiagara(const FVector& SpawnLocation) const
{
	if (AttackNiagaraId.IsValid())
	{
		NiagaraDelegate.ExecuteIfBound(AttackNiagaraId.PrimaryAssetName, SpawnLocation);
	}
}