#include "Character/Pet/PetCompanionAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"

APetCompanionAIController::APetCompanionAIController()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APetCompanionAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void APetCompanionAIController::SetPlayerTarget(APawn* NewOwnerPawn)
{
	if (NewOwnerPawn)
	{
		if (BehaviorTreeAsset)
		{

			// 현재 Blackboard는 TObjectPtr인데 `UseBlackboard`는 원시 포인터로 받기 때문에 타입을 임시로 변경
			UBlackboardComponent* TempBlackboard = Blackboard.Get();

			// 블랙보드 초기화 
			UseBlackboard(BehaviorTreeAsset->BlackboardAsset, TempBlackboard);

			// 블랙보드 OwnerActor를 Player로 설정한다
			GetBlackboardComponent()->SetValueAsObject(FName("OwnerActor"), NewOwnerPawn);

			// 행동 시작하기
			RunBehaviorTree(BehaviorTreeAsset);

			UE_LOG(LogTemp, Log, TEXT("Player를 변경: %s"), *NewOwnerPawn->GetName());
		}
	}
}


