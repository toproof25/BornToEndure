#include "Data/PetStatItemDataAsset.h"
#include "Component/PetItemComponent.h"
#include "Component/PetStatComponent.h"
#include "Stat/PetStatTypes.h"

void UPetStatItemDataAsset::ApplyToComponent(UPetItemComponent* ItemComp) const
{
	// 1.Owner의 StatComponent를 찾는다
	AActor* Owner = ItemComp->GetOwner();
	if (Owner == nullptr) return;
	UPetStatComponent* StatComp = Owner->FindComponentByClass<UPetStatComponent>();
	if (StatComp == nullptr) return;

	// 2.획득한 아이템의 FGuid를 가져온다
	FGuid SourceId = ItemComp->GetLastAddedInstanceId();

	// 3.해당 아이템에 대한 FGuid를 모두 추가한 후 StatComponent에 Modifier로 추가한다 (Stat 추적 가능하도록)
	for (FStatModifier Mod : StatModifiers)
	{
		Mod.SourceId = SourceId;
		StatComp->AddModifier(Mod);
	}
}

void UPetStatItemDataAsset::RemoveFromComponent(UPetItemComponent* ItemComp, const FGuid& InstanceId) const
{
	// 1.Owner의 StatComponent를 찾는다
	AActor* Owner = ItemComp->GetOwner();
	if (Owner == nullptr) return;
	UPetStatComponent* StatComp = Owner->FindComponentByClass<UPetStatComponent>();
	if (StatComp == nullptr) return;

	// 2.해당 아이템의 InstanceId를 보내서 StatComponent에서 제거한다
	StatComp->RemoveModifiersBySource(InstanceId);
}
