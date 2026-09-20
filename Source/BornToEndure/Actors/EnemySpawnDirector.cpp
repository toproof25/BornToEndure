#include "Actors/EnemySpawnDirector.h"
#include "Item/EnemySpawner.h"
#include "Kismet/GameplayStatics.h"
#include "Item/EnemySpawner.h"

AEnemySpawnDirector::AEnemySpawnDirector()
{
	PrimaryActorTick.bCanEverTick = false;

}


void AEnemySpawnDirector::StartWeaveSpawning()
{
	for (TObjectPtr<AEnemySpawner> Spawner : EnemySpawnPoints)
	{
		if (Spawner)
		{
			Spawner->StartWeaveSpawning();
		}
	}
}
void AEnemySpawnDirector::StopWeaveSpawning()
{
	for (TObjectPtr<AEnemySpawner> Spawner : EnemySpawnPoints)
	{
		if (Spawner)
		{
			Spawner->StopWeaveSpawning();
		}
	}
}

void AEnemySpawnDirector::BeginPlay()
{
	Super::BeginPlay();
	
	TArray<AActor*> TempEnemySpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemySpawner::StaticClass(), TempEnemySpawnPoints);
	for (AActor* Actor : TempEnemySpawnPoints)
	{
		if (Actor)
		{
			EnemySpawnPoints.Add(Cast<AEnemySpawner>(Actor));
		}
	}

}


