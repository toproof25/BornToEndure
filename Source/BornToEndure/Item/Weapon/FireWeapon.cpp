#include "Item/Weapon/FireWeapon.h"
#include "Data/DataTableRow/WeaponItemDataRow.h"
#include "Data/PetWeaponItemDataAsset.h"
#include "Data/CombatTypes.h"

void AFireWeapon::OnAttack(const FPetAttackInfo& AtkInfo)
{
	UE_LOG(LogBaseWeapon, Log, TEXT("AFireWeapon::OnAttack - FinalDamage: %f, IsCritical: %s, CriticalMultiplier: %f"), AtkInfo.FinalDamage, AtkInfo.bIsCritical ? TEXT("true") : TEXT("false"), AtkInfo.CriticalMultiplier);
}

void AFireWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void AFireWeapon::OnInitalizeWeapon(const UPetWeaponItemDataAsset& ItemData)
{
	// FireClass에 대한 데이터 설정
	UE_LOG(LogBaseWeapon, Log, TEXT("AFireWeapon::OnInitalizeWeapon"));	
}
