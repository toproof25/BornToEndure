#include "Component/PlayerHealthComponent.h"
#include "PlayerHealthComponent.h"

UPlayerHealthComponent::UPlayerHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}



void UPlayerHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
}

float UPlayerHealthComponent::HealthTakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead) return 0.0f;


	CurrentHealth -= DamageAmount;
	UE_LOG(LogTemp, Warning, TEXT("[UPlayerHealthComponent] HealthTakeDamage called. CurrentHealth: %f, DamageAmount: %f"), CurrentHealth, DamageAmount);

	if (CurrentHealth <= 0.0f)
	{
		bIsDead = true;
		CurrentHealth = 0.0f;
		OnPlayerDeath.Broadcast();
		UE_LOG(LogTemp, Warning, TEXT("[UPlayerHealthComponent] Player has died."));
	}

	return DamageAmount;
}

