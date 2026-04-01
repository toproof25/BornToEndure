// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "PetCompanionAIController.generated.h"

class APawn;
class UBehaviorTree;

UCLASS()
class BORNTOENDURE_API APetCompanionAIController : public AAIController
{
	GENERATED_BODY()
	

public:
	APetCompanionAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;

public:

	/**
	 * @brief 따라다닐 Player Pawn을 설정하는 함수
	 * @param NewOwnerPawn 새로 설정할 Player Pawn
	 */
	void SetPlayerTarget(APawn* NewOwnerPawn);

private:
	
	/**
	 * @brief AI 행동 트리 Behavior Tree Asset
	 */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;
};
