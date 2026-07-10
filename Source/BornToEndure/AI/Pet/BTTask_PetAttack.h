#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_PetAttack.generated.h"

UCLASS()
class BORNTOENDURE_API UBTTask_PetAttack : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_PetAttack();

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

public:

    /**
     * @brief BTService_PetRadar가 설정하는 TargetEnemy
     */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetEnemyKey;

    /**
     * @brief 공격 시 적을 향해 회전할지 여부
     */
    UPROPERTY(EditAnywhere, Category = "AI|Attack")
    bool bFaceTargetBeforeAttack = true;
};