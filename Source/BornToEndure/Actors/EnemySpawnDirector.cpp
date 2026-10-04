#include "Actors/EnemySpawnDirector.h"
#include "Item/EnemySpawner.h"
#include "Kismet/GameplayStatics.h"
#include "Item/EnemySpawner.h"
#include "Subsystem/ObjectPoolSubsystem.h"
#include "Character/Enemy/BaseEnemyCharacter.h"

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

void AEnemySpawnDirector::KillAllEnemies()
{
	UWorld* World = GetWorld();
	if (!World) return;
	UObjectPoolSubsystem* Pool = World->GetSubsystem<UObjectPoolSubsystem>(); 
	if (!Pool) return;

	TArray<AActor*> TempEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseEnemyCharacter::StaticClass(), TempEnemies);
	for (AActor* Actor : TempEnemies)
	{
		if (Actor)
		{
			Pool->ReturnPoolActor(Actor);
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


