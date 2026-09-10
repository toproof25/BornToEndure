#pragma once

#include "CoreMinimal.h"
#include "Data/PetItemDataAsset.h"
#include "UObject/PrimaryAssetId.h"

#include "PetWeaponItemDataAsset.generated.h"

class ABaseWeapon;
class UStaticMesh;

USTRUCT(BlueprintType)
struct BORNTOENDURE_API FWeaponPrimaryAssetIds
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "WeaponItem|Sound", meta = (AllowedTypes = "SoundDataAsset"))
	FPrimaryAssetId SpawnSoundId;
	UPROPERTY(EditDefaultsOnly, Category = "WeaponItem|Niagara", meta = (AllowedTypes = "NiagaraDataAsset"))
	FPrimaryAssetId SpawnNiagaraId;
	UPROPERTY(EditDefaultsOnly, Category = "WeaponItem|Sound", meta = (AllowedTypes = "SoundDataAsset"))
	FPrimaryAssetId AttackSoundId;
	UPROPERTY(EditDefaultsOnly, Category = "WeaponItem|Niagara", meta = (AllowedTypes = "NiagaraDataAsset"))
	FPrimaryAssetId AttackNiagaraId;
};

/**
 * @brief 실제로 인스턴스가 되며 공격을 변환하는 아이템 DataAsset
 */
UCLASS(BlueprintType)
class BORNTOENDURE_API UPetWeaponItemDataAsset : public UPetItemDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponItem Data")
	TSubclassOf<ABaseWeapon> WeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponItem Data")
	TSoftObjectPtr<UStaticMesh> WeaponStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponItem Data")
	FWeaponPrimaryAssetIds WeaponPrimaryAssetIds;

	virtual void ApplyToComponent(UPetItemComponent* ItemComp) const override;
	virtual void RemoveFromComponent(UPetItemComponent* ItemComp, const FGuid& InstanceId) const override;
};
