#include "Item/Weapon/FireWeapon.h"

#include "Data/DataTableRow/WeaponItemDataRow.h"
#include "Data/PetWeaponItemDataAsset.h"
#include "Data/PetFireWeaponItemDataAsset.h"
#include "Data/CombatTypes.h"

#include "Item/Projectile/BaseProjectile.h"
#include "Subsystem/ObjectPoolSubsystem.h"

void AFireWeapon::OnAttack(const FPetAttackInfo& AtkInfo, const FVector& TargetLocation)
{
	UE_LOG(LogBaseWeapon, Log, TEXT("AFireWeapon::OnAttack - FinalDamage: %f, IsCritical: %s, CriticalMultiplier: %f"), AtkInfo.FinalDamage, AtkInfo.bIsCritical ? TEXT("true") : TEXT("false"), AtkInfo.CriticalMultiplier);

	UWorld* World = GetWorld();
	if (!World) return;
	UObjectPoolSubsystem* PoolSubsystem = World->GetSubsystem<UObjectPoolSubsystem>();
	if (!PoolSubsystem) return;

	AActor* Projetile = PoolSubsystem->RequestPoolActor(ProjectileClass);
	if (!Projetile) return;
	ABaseProjectile* AttackProjectile = Cast<ABaseProjectile>(Projetile);
	if (!AttackProjectile) return;

	// 발사체에 대한 소유자와 주체자 초기화
	AttackProjectile->Owner = GetOwner();
	AttackProjectile->SetInstigator(Cast<APawn>(GetOwner()));
	//AttackProjectile->SetActorLocationAndRotation(Origin, Dir.Rotation(), false, nullptr, ETeleportType::TeleportPhysics);
	AttackProjectile->SetActorScale3D(FVector(ProjectileData.ProjectileSize));

	OnAttackSoundAndNiagara(GetActorLocation());
	AttackProjectile->FireProjectile(ProjectileData, AtkInfo, TargetLocation);
}

void AFireWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void AFireWeapon::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 사용하던 발사체 객체 제거
	if (UWorld* World = GetWorld())
	{
		if (UObjectPoolSubsystem* PoolSubsystem = World->GetSubsystem<UObjectPoolSubsystem>())
		{
			PoolSubsystem->RemovePoolActor(ProjectileClass);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AFireWeapon::OnInitalizeWeapon(const UPetWeaponItemDataAsset& ItemData)
{
	const UPetFireWeaponItemDataAsset* FireWeaponData = Cast<UPetFireWeaponItemDataAsset>(&ItemData);

	// FireClass에 대한 데이터 설정
	UE_LOG(LogBaseWeapon, Log, TEXT("AFireWeapon::OnInitalizeWeapon"));	

	SetProjectileClass(FireWeaponData->ProjectileClass);
	UpdateProjectileData(FireWeaponData->ProjectileData);
}

void AFireWeapon::SetProjectileClass(TSubclassOf<class ABaseProjectile> InProjectileClass)
{
	ProjectileClass = InProjectileClass;
	InitializeProjectilePool();
}

void AFireWeapon::UpdateProjectileData(const FProjectileData& InProjectileData)
{
	ProjectileData = InProjectileData;
}

void AFireWeapon::InitializeProjectilePool()
{
	UWorld* World = GetWorld();
	if (!World) return;
	UObjectPoolSubsystem* PoolSubsystem = World->GetSubsystem<UObjectPoolSubsystem>();
	if (!PoolSubsystem) return;

	PoolSubsystem->InitializePoolForClass(ProjectileClass, 20); // 예시로 20개의 발사체를 풀에 초기화
}
