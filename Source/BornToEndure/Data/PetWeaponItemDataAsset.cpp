#include "PetWeaponItemDataAsset.h"
#include "Component/PetItemComponent.h"


void UPetWeaponItemDataAsset::ApplyToComponent(UPetItemComponent* ItemComp) const
{
	ItemComp->AddWeaponModifier(WeaponModifier, ItemComp->GetLastAddedInstanceId());
}

void UPetWeaponItemDataAsset::RemoveFromComponent(UPetItemComponent* ItemComp, const FGuid& InstanceId) const
{
	ItemComp->RemoveWeaponModifier(InstanceId);
}
