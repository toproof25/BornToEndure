#include "AI/Pet/BTService_PetRadar.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Component/EnemyDetectorComponent.h"
#include "Character/Pet/PetCompanionCharacter.h"
#include "Character/Enemy/BaseEnemyCharacter.h"

UBTService_PetRadar::UBTService_PetRadar()
{
    NodeName = TEXT("Pet Radar");
    Interval = 0.1f;  
    RandomDeviation = 0.0f;
}

void UBTService_PetRadar::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon) return;

    APetCompanionCharacter* PetChar = Cast<APetCompanionCharacter>(AICon->GetPawn());
    if (!PetChar) return;

    UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
    if (!BBComp) return;

    AActor* OwnerActor = Cast<AActor>(BBComp->GetValueAsObject(OwnerActorKey.SelectedKeyName));
    if (!OwnerActor) return;

    const FVector PetLoc = PetChar->GetActorLocation();
    const FVector OwnerLoc = OwnerActor->GetActorLocation();

    // Player와 거리 계산 - 멀어지면 즉시 복귀
    const float DistToOwnerSq = FVector::DistSquared(PetLoc, OwnerLoc);
    const bool  bTooFar = DistToOwnerSq > FMath::Square(MaxDistanceToOwner);

    BBComp->SetValueAsBool(bIsTooFarKey.SelectedKeyName, bTooFar);

    if (bTooFar)
    {
        BBComp->ClearValue(TargetEnemyKey.SelectedKeyName);
        BBComp->SetValueAsBool(bInAttackRangeKey.SelectedKeyName, false);
        return;
    }

    // Player기준의 근방 공유 적 리스트 체크
    UEnemyDetectorComponent* DetectorComp = OwnerActor->FindComponentByClass<UEnemyDetectorComponent>();
    if (!DetectorComp)
    {
        BBComp->ClearValue(TargetEnemyKey.SelectedKeyName);
        BBComp->SetValueAsBool(bInAttackRangeKey.SelectedKeyName, false);
        return;
    }
    const TArray<TWeakObjectPtr<ABaseEnemyCharacter>>& EnemyList = DetectorComp->GetCachedEnemies();

    // 적 리스트 TArray를 바탕으로 Pet 기준 가장 가까운 적 Target 설정
    ABaseEnemyCharacter* ClosestEnemy = nullptr;
    float ClosestDistSq = FLT_MAX;

    for (const TWeakObjectPtr<ABaseEnemyCharacter>& EnemyPtr : EnemyList)
    {
        if (!EnemyPtr.IsValid()) continue;

        const float DistSq = FVector::DistSquaredXY(PetLoc, EnemyPtr->GetActorLocation());
        if (DistSq < ClosestDistSq)
        {
            ClosestDistSq = DistSq;
            ClosestEnemy = EnemyPtr.Get();
        }
    }

    // 블랙보드 갱신
    if (!ClosestEnemy)
    {
        BBComp->ClearValue(TargetEnemyKey.SelectedKeyName);
        BBComp->SetValueAsBool(bInAttackRangeKey.SelectedKeyName, false);
        return;
    }

    BBComp->SetValueAsObject(TargetEnemyKey.SelectedKeyName, ClosestEnemy);

    const float AttackRange = PetChar->GetAttackRange();
    const bool  bInRange = ClosestDistSq <= FMath::Square(AttackRange + 30.f);
    BBComp->SetValueAsBool(bInAttackRangeKey.SelectedKeyName, bInRange);

    // 접근 목표 위치 계산 (공격 범위 + Owner 반경 클램핑)
    const FVector NewApproachLoc = ComputeAttackApproachLocation(
        PetLoc,
        ClosestEnemy->GetActorLocation(),
        OwnerLoc,
        AttackRange,
        MaxDistanceToOwner
    );

    // 블랙보드에 저장된 현재 목표 위치를 가져옴
    const FVector CurrentApproachLoc = BBComp->GetValueAsVector(MoveTargetLocationKey.SelectedKeyName);

    // 일정 거리 이상인 경우에만 갱신
    if (FVector::DistSquaredXY(CurrentApproachLoc, NewApproachLoc) > FMath::Square(50.0f))
    {
        BBComp->SetValueAsVector(MoveTargetLocationKey.SelectedKeyName, NewApproachLoc);
    }
}

FVector UBTService_PetRadar::ComputeAttackApproachLocation(
    const FVector& PetLoc,
    const FVector& EnemyLoc,
    const FVector& OwnerLoc,
    float AttackRange,
    float MaxOwnerDist) const
{
    const FVector ToEnemy2D = (EnemyLoc - PetLoc).GetSafeNormal2D();

    FVector IdealPos = EnemyLoc - ToEnemy2D * (AttackRange * 0.85f);
    IdealPos.Z = PetLoc.Z;

    if (FVector::DistSquaredXY(IdealPos, OwnerLoc) <= FMath::Square(MaxOwnerDist))
    {
        return IdealPos;
    }

    const FVector OwnerToEnemy2D = (EnemyLoc - OwnerLoc).GetSafeNormal2D();
    FVector ClampedPos = OwnerLoc + OwnerToEnemy2D * (MaxOwnerDist * 0.95f);
    ClampedPos.Z = PetLoc.Z;

    return ClampedPos;
}