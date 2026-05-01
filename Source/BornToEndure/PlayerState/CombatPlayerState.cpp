
#include "PlayerState/CombatPlayerState.h"
#include "Data/GameTypes.h"

DEFINE_LOG_CATEGORY(LogCombatPlayerState);

void ACombatPlayerState::UpdateDamageStats(FName KillerPetName, const FPetDamageMap& PetDamageMap, float TotalDamageReceiced)
{
	for (auto& Pet : PetDamageMap)
	{
		PetDamageStats.FindOrAdd(Pet.Key) += Pet.Value;
		UE_LOG(LogCombatPlayerState, Log, TEXT("Pet %s dealt %f damage to player"), *Pet.Key.ToString(), Pet.Value);
	}
}
