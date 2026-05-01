#include "Component/PlayerExperienceComponent.h"
#include "Character/Enemy/BaseEnemyCharacter.h"
#include "Data/GameTypes.h"
#include "PlayerState/CombatPlayerState.h"

DEFINE_LOG_CATEGORY(LogPlayerExperienceComponent);

UPlayerExperienceComponent::UPlayerExperienceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerExperienceComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPlayerExperienceComponent::RegisterEnemyPayload(ABaseEnemyCharacter* Enemy)
{
	if (!Enemy)
	{
		UE_LOG(LogPlayerExperienceComponent, Warning, TEXT("Attempted to register a null enemy payload."));
		return;
	}

	// 지속적으로 구독과 취소가 반복될 때는 중복 바인딩 방지를 위해 제거 후 추가하는게 안전하다
	Enemy->OnEnemyKilled.RemoveDynamic(this, &UPlayerExperienceComponent::OnEnemyKilledHandler); 
	Enemy->OnEnemyKilled.AddDynamic(this, &UPlayerExperienceComponent::OnEnemyKilledHandler);
	UE_LOG(LogPlayerExperienceComponent, Log, TEXT("Registered enemy payload: %s"), *Enemy->GetName());
}

void UPlayerExperienceComponent::OnEnemyKilledHandler(const FEnemyRewardPayload& Payload)
{
	CurrentXP += Payload.ExpReward;
	CurrentGold += Payload.GoldReward;

	UE_LOG(LogPlayerExperienceComponent, Log, TEXT("Enemy killed! Gained %f XP and %d Gold. Total XP: %f, Total Gold: %d"),
		Payload.ExpReward, Payload.GoldReward, CurrentXP, CurrentGold);

	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (OwningPawn)
	{
		TObjectPtr<ACombatPlayerState> PS = OwningPawn->GetPlayerState<ACombatPlayerState>();
		if (PS)
		{
			// PlayerState로 통계 데이터 전송
			PS->UpdateDamageStats(Payload.KillerPetId, Payload.PetDamageMap, Payload.TotalDamageReceiced);
		}
	}

}
