// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Pet/BTTask_PetAttack.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BlackboardComponent.h" // 필수 헤더
#include "Subsystem/ObjectPoolSubsystem.h"
#include "AIController.h"
#include "Character/Pet/PetCompanionCharacter.h"

UBTTask_PetAttack::UBTTask_PetAttack()
{
	bNotifyTick = false;
	NodeName = "Pet Attack";
}

EBTNodeResult::Type UBTTask_PetAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UBTTask_PetAttack_Execute);

    AAIController* PetAICon = OwnerComp.GetAIOwner();
	if (PetAICon == nullptr) return EBTNodeResult::Failed;

	APawn* PetPawn = PetAICon->GetPawn();
	if (PetPawn == nullptr) return EBTNodeResult::Failed;

    APetCompanionCharacter* SelfPetCharacter = Cast<APetCompanionCharacter>(PetPawn);
	if (SelfPetCharacter == nullptr) return EBTNodeResult::Failed;

    UBlackboardComponent* PetBBComp = OwnerComp.GetBlackboardComponent();
	if (PetBBComp == nullptr) return EBTNodeResult::Failed;

	AActor* Enemy = Cast<AActor>(PetBBComp->GetValueAsObject(TargetEnemyKey.SelectedKeyName));
	if (Enemy == nullptr) return EBTNodeResult::Failed;

	// 공격 함수 호출
	SelfPetCharacter->OnAttack(Enemy->GetActorLocation());

	//sUE_LOG(LogTemp, Warning, TEXT("DetectEnemy Service Pet Attack! Target: %s"), *Enemy->GetName());

	return EBTNodeResult::Succeeded;
}

void UBTTask_PetAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
}
