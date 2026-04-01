#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PetCompanionCharacter.generated.h"

class USphereComponent;
class UPetCombatComponent;

UCLASS()
class BORNTOENDURE_API APetCompanionCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APetCompanionCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;


public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UPetCombatComponent> PetCombatComp;

	/**
	 * @brief 적 탐지 충돌체 컴포넌트
	 * @note 현재는 사용할 수 있으나 추후에는 다른 방식으로 변경할 예정
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> DetectionSphere;

	/**
	 * @brief PetCombatComp의 공격 함수를 위임한 함수
	 * @param TargetVector 공격할 적 위치
	 * @note UBTTask_PetAttack에서 호출됩니다
	 */
	void OnAttack(const FVector& TargetVector);

};
