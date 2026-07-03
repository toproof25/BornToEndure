/**
* @file PlayerHealthComponent.h
* @data 2026-06-29
* @author toproof
* @brief 플레이어의 체력 관리를 위한 컴포넌트 클래스
* - 플레이어의 체력 관리
* - 체력 회복 및 데미지 처리
* - 사망 시 Delegate 처리
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Delegates/Delegate.h"

#include "PlayerHealthComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BORNTOENDURE_API UPlayerHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPlayerHealthComponent();

	/**
	 * @brief PlayerCharacter에서 TakeDamage를 실질적으로 처리하는 함수로 위임이 되어 호출이 된다
	 * @param DamageAmount 데미지 수치
	 * @param DamageEvent 데미지 이벤트 구조체
	 * @param EventInstigator 공격자 컨트롤러
	 * @param DamageCauser 공격자 액터
	 * @return 실질적으로 받은 데미지 (최종 데미지)
	 */
	float HealthTakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser);

protected:
	virtual void BeginPlay() override;

private:

	UPROPERTY(EditAnywhere, Category = "Health")
	float MaxHealth = 100.f;
	UPROPERTY(VisibleAnywhere, Category = "Health")
	float CurrentHealth;

	bool bIsDead = false;

public:

	//void Heal(float HealAmount);
	float GetCurrentHealth() const { return CurrentHealth; }
	float GetMaxHealth() const { return MaxHealth; }

	DECLARE_MULTICAST_DELEGATE(FOnPlayerDeath)
	FOnPlayerDeath OnPlayerDeath;
		
};
