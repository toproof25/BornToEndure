#include "AI/Pet/BTTask_MoveToAttackPosition.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_MoveToAttackPosition::UBTTask_MoveToAttackPosition()
{
    NodeName = TEXT("Move To Attack Position");
    bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_MoveToAttackPosition::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon) return EBTNodeResult::Failed;

    UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
    if (!BBComp) return EBTNodeResult::Failed;

    const FVector TargetLoc = BBComp->GetValueAsVector(MoveTargetLocationKey.SelectedKeyName);

    // AI 이동 명령 — 결과는 TickTask에서 지속 확인
    const FAIMoveRequest MoveReq = [&]()
        {
            FAIMoveRequest Req;
            Req.SetGoalLocation(TargetLoc);
            Req.SetAcceptanceRadius(AcceptanceRadius);
            Req.SetUsePathfinding(true);
            return Req;
        }();

    AICon->MoveTo(MoveReq);
    return EBTNodeResult::InProgress;
}

void UBTTask_MoveToAttackPosition::TickTask(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // 이동 경로 추적 컴포넌트 상태 확인
    UPathFollowingComponent* PFC = AICon->GetPathFollowingComponent();
    if (!PFC)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    const EPathFollowingStatus::Type Status = PFC->GetStatus();

    if (Status == EPathFollowingStatus::Idle)
    {
        // 이동 완료 또는 경로 없음 → BT가 다음 틱에 재평가
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    // 서비스가 갱신한 새 목표 위치로 지속 재요청 (적 이동 추적)
    UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
    if (BBComp)
    {
        const FVector NewTargetLoc = BBComp->GetValueAsVector(MoveTargetLocationKey.SelectedKeyName);
        FAIMoveRequest Req;
        Req.SetGoalLocation(NewTargetLoc);
        Req.SetAcceptanceRadius(AcceptanceRadius);
        Req.SetUsePathfinding(true);
        AICon->MoveTo(Req);
    }
}

EBTNodeResult::Type UBTTask_MoveToAttackPosition::AbortTask(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (AAIController* AICon = OwnerComp.GetAIOwner())
    {
        AICon->StopMovement();
    }
    return EBTNodeResult::Aborted;
}