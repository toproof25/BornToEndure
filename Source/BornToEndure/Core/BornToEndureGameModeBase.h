/**
* @file BornToEndureGameModeBase.h
* @brief BornToEndure의 게임 플레이를 관리하는 GameModeBase 클래스
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BornToEndureGameModeBase.generated.h"

class AEnemySpawnDirector;

UENUM(BlueprintType)
enum EGameState : uint8
{
	// 게임 시작 전 상태, 시작 상태, 게임 오버 상태, 퍼즈 상태에 대한 열거형 정의
	WatingToStart UMETA(DisplayName = "WaitingToStart"),
	Playing UMETA(DisplayName = "Playing"),
	Paused UMETA(DisplayName = "Paused"),
	GameOver UMETA(DisplayName = "GameOver")

};

UCLASS()
class BORNTOENDURE_API ABornToEndureGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	void StartGame();
	void EndGame();

	void StartWave();
	void EndWave();
	
	void SetSpawnEnemies();

	int32 GetCurrentWave() const { return CurrentWave; }
	EGameState GetGameState() const { return GameState; }
	double GetStartPlayTime() const { return StartPlayTime; }
	const FTimerHandle& GetWaveTimerHandle() const { return WaveTimerHandle; }

protected:
	virtual void BeginPlay() override;

private:

	int32 CurrentWave;
	EGameState GameState;
	double StartPlayTime;
	//double CurrentPlayTime;

	TObjectPtr<AEnemySpawnDirector> EnemySpawnDirector;

	FTimerHandle WaveTimerHandle;
};
