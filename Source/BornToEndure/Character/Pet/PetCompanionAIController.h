// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "PetCompanionAIController.generated.h"


class APawn;
struct FTimerHandle;

UCLASS()
class BORNTOENDURE_API APetCompanionAIController : public AAIController
{
	GENERATED_BODY()
	

public:
	APetCompanionAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;


public:
	void SetPlayerTarget(APawn* NewOwnerPawn);

private:

	// 이동을 담당하는 함수
	void UpdateRandomMoveLocation();

	// 타이머 핸들러
	FTimerHandle MoveTimerHandle;

	// 추적할 플레이어의 포인터 캐싱
	UPROPERTY()
	TObjectPtr<APawn> PlayerPawn;

	// 설정값
	float MinRadius = 200.0f;           // 플레이어와 너무 가깝지 않게 유지할 최소 거리
	float MaxRadius = 500.0f;           // 플레이어 주변에서 돌아다닐 최대 반경
	float ReturnRadiusSquared = 1000000.0f; // 1000^2 (강제 복귀 거리의 제곱)
	float MoveInterval = 3.0f;          // 몇 초마다 새로운 장소를 찾을 것인가?

};
