// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#if !UE_BUILD_SHIPPING   
	#include "ImGuiDelegates.h" 
#endif

#include "Test_BTSandBTTask_BP_CPP.generated.h"


class ABaseEnemy;

UCLASS()
class BORNTOENDURE_API ATest_BTSandBTTask_BP_CPP : public AActor
{
	GENERATED_BODY()
	
public:	
	ATest_BTSandBTTask_BP_CPP();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseEnemy> EnemyClass;

	FTimerHandle FireTimerHandle;        // 타이머 핸들
	FImGuiDelegateHandle ImGuiDelegateHandle;

	void RenderImGui();
	void OnTestSpawnEnemy();

	// 🌟 스폰된 적을 기억해두고 나중에 파괴하기 위한 포인터
	UPROPERTY()
	TArray<AActor*> SpawnedEnemies;

	AActor* LoadEnemy;

private:
	void OnTestEnd();
};
