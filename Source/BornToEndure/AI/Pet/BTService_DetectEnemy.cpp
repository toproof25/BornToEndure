// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Pet/BTService_DetectEnemy.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Engine/OverlapResult.h"
#include "Character/Enemy/BaseEnemy.h"

UBTService_DetectEnemy::UBTService_DetectEnemy()
{
	NodeName = TEXT("Detect Enemy And Check Owner");
}

void UBTService_DetectEnemy::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);


	// AI Controller로 Pet Pawn 가져오기
	AAIController* PetAICon = OwnerComp.GetAIOwner();
	if (PetAICon == nullptr) return;

	APawn* PetPawn = PetAICon->GetPawn();
	if (PetPawn == nullptr) return;

	// 현재 Pet Blackboard 가져오기
	UBlackboardComponent* PetBBComp = OwnerComp.GetBlackboardComponent();
	if (PetBBComp == nullptr) return;

	// Pet이 따르는 Player 가져오기
	AActor* OwnerActor = Cast<AActor>(PetBBComp->GetValueAsObject(OwnerActorKey.SelectedKeyName));
	if (OwnerActor == nullptr) return;

	// Player와 Pet의 거리 계산
	FVector PetLoc = PetPawn->GetActorLocation();
	FVector OwnerLoc = OwnerActor->GetActorLocation();

	float DistToOwnerSq = FVector::DistSquared(PetLoc, OwnerLoc);
	float MaxDistSq = MaxDistanceToOwner * MaxDistanceToOwner;

	if (DistToOwnerSq > MaxDistSq)
	{
		// 너무 멀어진 경우 즉시 Player에게 돌아가기
		PetBBComp->SetValueAsBool(bIsTooFarKey.SelectedKeyName, true);
		PetBBComp->ClearValue(TargetEnemyKey.SelectedKeyName);
		return;
	}
	else
	{
		PetBBComp->SetValueAsBool(bIsTooFarKey.SelectedKeyName, false);
	}

	// ---------------------------------------------------------
	// 적 탐지 (OverlapMultiByChannel)
	// ---------------------------------------------------------

	// 기존 Capacity는 유지한 채 비우기
	OverlapResults.Reset();

	FCollisionShape Sphere = FCollisionShape::MakeSphere(DetectRadius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(PetPawn);   
	Params.AddIgnoredActor(OwnerActor);

	// 추후 Enemy 채널 만들어서 사용하기
	bool bHit = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		PetLoc,
		FQuat::Identity,
		ECC_GameTraceChannel3, // Enemy 채널
		Sphere,
		Params
	);

	AActor* ClosestEnemy = nullptr;
	float ClosestDistSq = FMath::Square(DetectRadius); // 초기 최솟값을 최대 탐지 반경으로 설정

	if (bHit)
	{
		for (const FOverlapResult& Result : OverlapResults)
		{
			AActor* HitActor = Result.GetActor();
			ABaseEnemy* Enemy = Cast<ABaseEnemy>(HitActor);
			if (Enemy)
			{
				float DistSq = FVector::DistSquared(PetLoc, HitActor->GetActorLocation());
				if (DistSq < ClosestDistSq)
				{
					ClosestDistSq = DistSq;
					ClosestEnemy = HitActor;
				}
			}
		}
	}

	// 탐지 결과 블랙보드에 갱신
	if (ClosestEnemy)
	{
		PetBBComp->SetValueAsObject(TargetEnemyKey.SelectedKeyName, ClosestEnemy);
	}
	else
	{
		PetBBComp->ClearValue(TargetEnemyKey.SelectedKeyName);
	}

}