#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"

#include "BTService_PetRadar.generated.h"

/**
 * 매 Service 틱마다:
 *   1. Player와 거리 검사 -> bIsTooFar 갱신
 *   2. Player의 UEnemyDetectorComponent 캐시에서 가장 가까운 적 탐색
 *   3. TargetEnemy, bInAttackRange, MoveTargetLocation 갱신
 *
 * Pet의 공격 범위는 APetCompanionCharacter::GetAttackRange()로 런타임에 읽는다.
 */
UCLASS()
class BORNTOENDURE_API UBTService_PetRadar : public UBTService_BlackboardBase
{
    GENERATED_BODY()

public:
    UBTService_PetRadar();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
    /** 공격 가능 위치를 계산한다 (Player 반경 클램핑 포함) */
    FVector ComputeAttackApproachLocation(
        const FVector& PetLoc,
        const FVector& EnemyLoc,
        const FVector& OwnerLoc,
        float AttackRange,
        float MaxOwnerDist
    ) const;

public:
    // ─── Blackboard Keys ──────────────────────────────────────────
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetEnemyKey;         // Object

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector OwnerActorKey;           // Object

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector bIsTooFarKey;            // Bool

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector bInAttackRangeKey;       // Bool

    /** 적 방향으로 접근할 목표 위치 — BTTask_MoveToAttackPosition 이 읽는다 */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector MoveTargetLocationKey;   // Vector

    // ─── Radar Parameters ─────────────────────────────────────────
    /** Pet이 이 거리를 초과하면 Player에게 복귀 */
    UPROPERTY(EditAnywhere, Category = "AI|Radar")
    float MaxDistanceToOwner = 1000.0f;
};