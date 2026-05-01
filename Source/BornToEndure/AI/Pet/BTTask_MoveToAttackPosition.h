#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MoveToAttackPosition.generated.h"

/**
 * @brief Service가 계산한 MoveTargetLocation으로 이동
 * - 도달 판정: Pet과 목표 위치의 거리가 AcceptanceRadius 이하
 * - 이동 중 서비스가 위치를 갱신하므로 자연스럽게 추적
 */
UCLASS()
class BORNTOENDURE_API UBTTask_MoveToAttackPosition : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_MoveToAttackPosition();

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

public:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector MoveTargetLocationKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector OwnerActorKey;

    /**
     * @brief 목표 도달 판정 반경 수치
     */
    UPROPERTY(EditAnywhere, Category = "AI|Movement")
    float AcceptanceRadius = 60.0f;

    /** Player 반경 초과 시 이동 중단 여부 */
    UPROPERTY(EditAnywhere, Category = "AI|Movement")
    float MaxDistanceToOwner = 1000.0f;
};