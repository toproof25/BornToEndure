// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_PetAttack.generated.h"


struct FBlackboardKeySelector;

/**
 * 
 */
UCLASS()
class BORNTOENDURE_API UBTTask_PetAttack : public UBTTaskNode
{
	GENERATED_BODY()
	

public:	
	UBTTask_PetAttack();

protected:
	
	// 노드가 실행될 때 호출된다
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// 행동이 Frame 단위로 업데이트가 필요할 때
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

public:
	/**
	 * @brief 공격할 Blackboard Target Key
	 * @note 상위 노드의 Services에서 TargetEnemyKey에 값을 설정됩니다
	 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetEnemyKey;

	/**
	 * @brief 자신이 따라다니는 주인 Actor Key
	 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector OwnerActorKey;

};
