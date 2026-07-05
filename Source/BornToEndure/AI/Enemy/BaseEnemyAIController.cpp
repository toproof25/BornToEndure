#include "AI/Enemy/BaseEnemyAIController.h"

#include "AIController.h"
#include "Character/Enemy/BaseEnemyCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"

void ABaseEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    ABaseEnemyCharacter* Enemy = Cast<ABaseEnemyCharacter>(InPawn);
    if (Enemy && Enemy->GetEnemyBehaviorTree())
    {
        RunBehaviorTree(Enemy->GetEnemyBehaviorTree());
    }
}

void ABaseEnemyAIController::SetTargetPlayer(APawn* Player)
{
    if (GetBlackboardComponent())
    {
		GetBlackboardComponent()->SetValueAsObject(FName("TargetActor"), Player);
    }
}
