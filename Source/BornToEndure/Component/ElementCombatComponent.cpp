#include "Component/ElementCombatComponent.h"
#include "GameplayTagContainer.h"
#include "Data/DataTableRow/EnemyDataRow.h"

UElementCombatComponent::UElementCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UElementCombatComponent::InitializeElementResistance(const TArray<FEnemyElementResistanceEntry>& ElementResistances)
{
	// 각 배열을 순회하면서 TMap에 추가함
	for (const FEnemyElementResistanceEntry& ElementEntry : ElementResistances)
	{
		ElementResistanceMap.FindOrAdd(ElementEntry.ElementTag) = ElementEntry.ResistanceValue;
	}

	// 테스트용 TMap 로그 출력
	for (const auto& Pair : ElementResistanceMap)
	{
		UE_LOG(LogTemp, Log, TEXT("[UElementCombatComponent] Element: %s, Resistance: %.2f"), *Pair.Key.ToString(), Pair.Value);
	}
}

float UElementCombatComponent::CalculateDamageWithElementResistance(float BaseDamage, const FGameplayTag& ElementTag) const
{
	if (const float* Resistance = ElementResistanceMap.Find(ElementTag))
	{
		UE_LOG(LogTemp, Log, TEXT("[UElementCombatComponent] Element: %s, Resistance: %.2f, BaseDamage: %.2f, FinalDamage: %.2f"), *ElementTag.ToString(), *Resistance, BaseDamage, BaseDamage * (1.0f - *Resistance));
		return BaseDamage * (1.0f - *Resistance);
	}
	
	UE_LOG(LogTemp, Log, TEXT("[UElementCombatComponent] Element: %s not found in resistance map. BaseDamage: %.2f"), *ElementTag.ToString(), BaseDamage);
	return BaseDamage;
}
