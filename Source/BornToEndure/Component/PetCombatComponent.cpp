#include "Component/PetCombatComponent.h"
#include "Subsystem/EffectSubsystem.h"
#include "Subsystem/ObjectPoolSubsystem.h"
#include "Delegates/Delegate.h"
#include "Item/Projectile/BaseProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"

UPetCombatComponent::UPetCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UPetCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	check(World);
	UEffectSubsystem* EffectSubsystem = GetWorld()->GetSubsystem<UEffectSubsystem>();
	check(EffectSubsystem);

	SoundDelegate.BindUObject(EffectSubsystem, &UEffectSubsystem::SpawnSoundAtLocation);
	NiagaraDelegate.BindUObject(EffectSubsystem, &UEffectSubsystem::SpawnNiagaraAtLocation);

	InitializeProjectilePool();
}

void UPetCombatComponent::OnAttack(const FVector& TargetVector)
{
	if (ProjectileClass == nullptr) return;

	// 오브젝트 풀 시스템 가져오기
	UWorld* World = GetWorld();
	if (World == nullptr) return;
	UObjectPoolSubsystem* ObjectPoolSubsystem = World->GetSubsystem<UObjectPoolSubsystem>();
	
	// 사용할 발사체 오브젝트 풀에서 가져오기
	UClass* ProjectileClassKey = ProjectileClass.Get();
	AActor* GetPoolActor = ObjectPoolSubsystem->RequestPoolActor(ProjectileClassKey);
	if (GetPoolActor == nullptr) return;
	ABaseProjectile* Projectile = Cast<ABaseProjectile>(GetPoolActor);
	if (Projectile == nullptr) return;

	AActor* OwnerActor = GetOwner();

	// 발사체 스폰 위치와 회전 설정
	FVector SpawnLocation = OwnerActor->GetActorLocation();

	// 적 위치에 대한 방향과 회전 설정
	FVector Direction = (TargetVector - SpawnLocation).GetSafeNormal();
	FRotator TargetRotation = Direction.Rotation();

	// 사운드 및 Niagara 재생
	OnAttackSound(SpawnLocation);
	OnAttackNiagara(SpawnLocation);

	// 발사체에 대한 소유자와 주체자 초기화
	Projectile->Owner = OwnerActor;
	Projectile->SetInstigator(OwnerActor->GetInstigator());

	// 이동 시 발사체가 충돌하지 않도록 위치와 회전을 설정 (위치 초기화)
	Projectile->SetActorLocationAndRotation(SpawnLocation, TargetRotation, false, nullptr, ETeleportType::TeleportPhysics);
	
	// 발사체 발사
	Projectile->FireProjectile(Direction);
}


void UPetCombatComponent::OnAttackSound(const FVector& SpawnLocation) const
{
	if (AttackSoundId.IsValid())
	{
		SoundDelegate.ExecuteIfBound(AttackSoundId.PrimaryAssetName, SpawnLocation);
	}
}

void UPetCombatComponent::OnAttackNiagara(const FVector& SpawnLocation) const
{
	if (AttackNiagaraId.IsValid())
	{
		NiagaraDelegate.ExecuteIfBound(AttackNiagaraId.PrimaryAssetName, SpawnLocation);
	}
}

void UPetCombatComponent::InitializeProjectilePool()
{
	if (ProjectilePoolSize <= 0) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UObjectPoolSubsystem* ObjectPoolSubsystem = World->GetSubsystem<UObjectPoolSubsystem>();
	if (ObjectPoolSubsystem)
	{
		UClass* Projectile = ProjectileClass.Get();
		ObjectPoolSubsystem->InitializePoolForClass(ProjectileClass, ProjectilePoolSize);
	}
}