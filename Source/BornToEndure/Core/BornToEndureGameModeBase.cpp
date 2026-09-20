#include "Core/BornToEndureGameModeBase.h"
#include "BornToEndureGameModeBase.h"
#include "Actors/EnemySpawnDirector.h"
#include "Kismet/GameplayStatics.h"

void ABornToEndureGameModeBase::StartGame()
{
	UE_LOG(LogTemp, Warning, TEXT("[ABornToEndureGameModeBase] 게임 시작!"));
	SpawnWave();
}

void ABornToEndureGameModeBase::SpawnWave()
{
	UE_LOG(LogTemp, Warning, TEXT("[ABornToEndureGameModeBase] 웨이브 생성!"));

	if (EnemySpawnDirector)
	{
		EnemySpawnDirector->StartWeaveSpawning();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ABornToEndureGameModeBase] EnemySpawnDirector가 설정되지 않았습니다."));
	}
}

void ABornToEndureGameModeBase::EndGame()
{
	UE_LOG(LogTemp, Warning, TEXT("[ABornToEndureGameModeBase] 게임 종료!"));
	if (EnemySpawnDirector)
	{
		EnemySpawnDirector->StopWeaveSpawning();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ABornToEndureGameModeBase] EnemySpawnDirector가 설정되지 않았습니다."));
	}
}

void ABornToEndureGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	EnemySpawnDirector = Cast<AEnemySpawnDirector>(UGameplayStatics::GetActorOfClass(GetWorld(), AEnemySpawnDirector::StaticClass()));
	if (EnemySpawnDirector)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ABornToEndureGameModeBase] EnemySpawnDirector가 설정되었습니다."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ABornToEndureGameModeBase] EnemySpawnDirector가 설정되지 않았습니다."));
	}

}
