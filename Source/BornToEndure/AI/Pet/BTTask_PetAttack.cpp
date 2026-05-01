#include "AI/Pet/BTTask_PetAttack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Pet/PetCompanionCharacter.h"
#include "Component/PetCombatComponent.h"

UBTTask_PetAttack::UBTTask_PetAttack()
{
    bNotifyTick = true;
    NodeName = TEXT("Pet Attack");
}

EBTNodeResult::Type UBTTask_PetAttack::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon) return EBTNodeResult::Failed;

    APetCompanionCharacter* PetChar = Cast<APetCompanionCharacter>(AICon->GetPawn());
    if (!PetChar) return EBTNodeResult::Failed;

    UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
    if (!BBComp) return EBTNodeResult::Failed;

    AActor* Enemy = Cast<AActor>(BBComp->GetValueAsObject(TargetEnemyKey.SelectedKeyName));
    if (!IsValid(Enemy)) return EBTNodeResult::Failed;

    // CombatComponent에 공격 타겟 설정, 타이머는 CombatComponent가 관리
    UPetCombatComponent* CombatComp = PetChar->GetCombatComponent();
    if (CombatComp)
    {
        CombatComp->SetAttackTarget(Enemy);
    }

    // Task는 타겟이 유효한 동안 계속 점유 - 죽거나 범위 이탈 시 해제됨
    return EBTNodeResult::InProgress;
}

void UBTTask_PetAttack::TickTask(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
    if (!BBComp)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // 타겟이 Blackboard에서 사라졌으면 공격 중지
    AActor* Enemy = Cast<AActor>(BBComp->GetValueAsObject(TargetEnemyKey.SelectedKeyName));
    if (!IsValid(Enemy))
    {
        if (AAIController* AICon = OwnerComp.GetAIOwner())
        {
            if (APetCompanionCharacter* PetChar = Cast<APetCompanionCharacter>(AICon->GetPawn()))
            {
                if (UPetCombatComponent* CombatComp = PetChar->GetCombatComponent())
                {
                    CombatComp->ClearAttackTarget();
                }
            }
        }
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    // 타겟이 바뀌었으면 교체 (BTService_PetRadar가 더 가까운 적으로 갱신한 경우)
    if (AAIController* AICon = OwnerComp.GetAIOwner())
    {
        if (APetCompanionCharacter* PetChar = Cast<APetCompanionCharacter>(AICon->GetPawn()))
        {
            if (UPetCombatComponent* CombatComp = PetChar->GetCombatComponent())
            {
                if (CombatComp->GetCurrentTarget() != Enemy)
                {
                    CombatComp->SetAttackTarget(Enemy);
                }
            }
        }
    }
}

EBTNodeResult::Type UBTTask_PetAttack::AbortTask(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // 복귀 등으로 Task가 강제 중단될 때 반드시 공격 중지
    if (AAIController* AICon = OwnerComp.GetAIOwner())
    {
        if (APetCompanionCharacter* PetChar = Cast<APetCompanionCharacter>(AICon->GetPawn()))
        {
            if (UPetCombatComponent* CombatComp = PetChar->GetCombatComponent())
            {
                CombatComp->ClearAttackTarget();
            }
        }
    }
    return EBTNodeResult::Aborted;
}