// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "Engine/OverlapResult.h"
#include "BTService_DetectEnemy.generated.h"



/**
 * 
 */
UCLASS()
class BORNTOENDURE_API UBTService_DetectEnemy : public UBTService_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTService_DetectEnemy();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

public:

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetEnemyKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector OwnerActorKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector bIsTooFarKey;


	// 적 탐지 반경
	UPROPERTY(EditAnywhere, Category = "AI|Radar")
	float DetectRadius = 1000.0f;

	// Player와 최대 거리
	UPROPERTY(EditAnywhere, Category = "AI|Radar")
	float MaxDistanceToOwner = 1200.0f;

private:
	TArray<FOverlapResult> OverlapResults;
};