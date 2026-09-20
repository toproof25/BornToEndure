/**
* @file EnemySpawnDirector.h
* @brief 적 스포너를 관리하는 클래스
*/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawnDirector.generated.h"

class AEnemySpawner;

UCLASS()
class BORNTOENDURE_API AEnemySpawnDirector : public AActor
{
	GENERATED_BODY()
	
public:	
	AEnemySpawnDirector();

	void StartWeaveSpawning();
	void StopWeaveSpawning();

protected:
	virtual void BeginPlay() override;

private:

	TArray<TObjectPtr<AEnemySpawner>> EnemySpawnPoints;

};
