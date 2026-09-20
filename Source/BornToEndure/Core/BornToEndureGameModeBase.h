/**
* @file BornToEndureGameModeBase.h
* @brief BornToEndure의 게임 플레이를 관리하는 GameModeBase 클래스
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BornToEndureGameModeBase.generated.h"

class AEnemySpawnDirector;

UCLASS()
class BORNTOENDURE_API ABornToEndureGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	void StartGame();

	void SpawnWave();

	void EndGame();

protected:
	virtual void BeginPlay() override;

private:

	int32 CurrentWave;
	float CurrentPlayTime;

	TObjectPtr<AEnemySpawnDirector> EnemySpawnDirector;


};
