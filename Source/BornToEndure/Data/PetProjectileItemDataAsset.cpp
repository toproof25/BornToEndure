#include "Data/PetProjectileItemDataAsset.h"
#include "Component/PetItemComponent.h"

void UPetProjectileItemDataAsset::ApplyToComponent(UPetItemComponent* ItemComp) const
{
	ItemComp->AddProjectileModifier(ProjectileModifier, ItemComp->GetLastAddedInstanceId());
}

void UPetProjectileItemDataAsset::RemoveFromComponent(UPetItemComponent* ItemComp, const FGuid& InstanceId) const
{
	ItemComp->RemoveProjectileModifier(InstanceId);
}
