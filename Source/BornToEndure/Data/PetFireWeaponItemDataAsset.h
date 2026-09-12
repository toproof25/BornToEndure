/**
* @file PetFireWeaponItemDataAsset.h
* @brief UPetFireWeaponItemDataAsset 클래스 정의
* - WeaponItemDataAsset의 파생 클래스이며, 발사체 데이터를 포함한다
*/

#pragma once

#include "CoreMinimal.h"
#include "Data/PetWeaponItemDataAsset.h"
#include "Data/CombatTypes.h"
#include "PetFireWeaponItemDataAsset.generated.h"

class ABaseProjectile;

UCLASS()
class BORNTOENDURE_API UPetFireWeaponItemDataAsset : public UPetWeaponItemDataAsset
{
	GENERATED_BODY()
	

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponItem Data|Projectile", meta = (AllowedTypes = "BaseProjectile"))
	TSubclassOf<class ABaseProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponItem Data|Projectile")
	FProjectileData ProjectileData;


};
