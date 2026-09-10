#include "Item/Weapon/BaseWeapon.h"

#include "Subsystem/EffectSubsystem.h"
#include "Data/DataTableRow/WeaponItemDataRow.h"
#include "Data/PetWeaponItemDataAsset.h"
#include "UObject/PrimaryAssetId.h"

DEFINE_LOG_CATEGORY(LogBaseWeapon);

ABaseWeapon::ABaseWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponStaticMesh"));
	RootComponent = WeaponStaticMesh;

	// 현재는 Mesh를 소유하지만, 추후 Actor Component로 변경할 수 있기에 없는 것 처럼 설정
	WeaponStaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponStaticMesh->SetSimulatePhysics(false);
	WeaponStaticMesh->SetCollisionProfileName(TEXT("IgnoreAll"));
	WeaponStaticMesh->SetVisibility(false);
}

void ABaseWeapon::InitializeWeapon(const UPetWeaponItemDataAsset& ItemData)
{
	WeaponStaticMesh->SetStaticMesh(ItemData.WeaponStaticMesh.LoadSynchronous());
	WeaponStaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponStaticMesh->SetSimulatePhysics(false);
	WeaponStaticMesh->SetCollisionProfileName(TEXT("IgnoreAll"));
	WeaponStaticMesh->SetVisibility(false);

	SpawnSoundId = ItemData.WeaponPrimaryAssetIds.SpawnSoundId;
	SpawnNiagaraId = ItemData.WeaponPrimaryAssetIds.SpawnNiagaraId;
	AttackSoundId = ItemData.WeaponPrimaryAssetIds.AttackSoundId;
	AttackNiagaraId = ItemData.WeaponPrimaryAssetIds.AttackNiagaraId;

	// 자식 클래스 데이터 설정
	OnInitalizeWeapon(ItemData);
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

void ABaseWeapon::OnSpawnSoundAndNiagara(const FVector& SpawnLocation) const
{
	UWorld* World = GetWorld();
	if (!World) return;
	UEffectSubsystem* EffectSubsystem = World->GetSubsystem<UEffectSubsystem>();
	if (!EffectSubsystem) return;

	EffectSubsystem->SpawnSoundAtLocation(SpawnSoundId.PrimaryAssetName, SpawnLocation);
	EffectSubsystem->SpawnNiagaraAtLocation(SpawnNiagaraId.PrimaryAssetName, SpawnLocation);
}

void ABaseWeapon::OnAttackSoundAndNiagara(const FVector& SpawnLocation) const
{
	UWorld* World = GetWorld();
	if (!World) return;
	UEffectSubsystem* EffectSubsystem = World->GetSubsystem<UEffectSubsystem>();
	if (!EffectSubsystem) return;

	EffectSubsystem->SpawnSoundAtLocation(AttackSoundId.PrimaryAssetName, SpawnLocation);
	EffectSubsystem->SpawnNiagaraAtLocation(AttackNiagaraId.PrimaryAssetName, SpawnLocation);
}
