#include "Actors/EnemySpawnDirector.h"
#include "Item/EnemySpawner.h"
#include "Kismet/GameplayStatics.h"
#include "Item/EnemySpawner.h"
#include "Subsystem/ObjectPoolSubsystem.h"
#include "Character/Enemy/BaseEnemyCharacter.h"

#include "Engine/AssetManager.h"
#include "Engine/DataAsset.h"
#include "Engine/StreamableManager.h"
#include "UObject/PrimaryAssetId.h"

#include "Data/DataTableRow/EnemyDataRow.h"
#include "Data/DataTableRow/SpawnEnemyDataRow.h"

AEnemySpawnDirector::AEnemySpawnDirector() { PrimaryActorTick.bCanEverTick = false; }


void AEnemySpawnDirector::StartWeaveSpawning()
{
	if (CachedEnemyRowDatas.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AEnemySpawnDirector] 스폰할 적 데이터가 없습니다."));
		return;
	}

	for (TObjectPtr<AEnemySpawner> Spawner : EnemySpawnPoints)
	{
		if (Spawner)
		{
			Spawner->StartWeaveSpawning(CachedEnemyRowDatas[0]);
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

void AEnemySpawnDirector::SetSpawnEnemies(int32 WaveNumber)
{	
	if (!SpawnEnemyDataTable || !EnemyDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AEnemySpawnDirector] SpawnEnemyDataTable 또는 EnemyDataTable이 설정되지 않았습니다."));
		return;
	}

	// 스폰할 적 데이터를 가져와서 설정
	const FName WaveRowName(*FString::Printf(TEXT("Wave_%02d"), WaveNumber));
	FSpawnEnemyDataRow* SpawnEnemyDataRow = SpawnEnemyDataTable->FindRow<FSpawnEnemyDataRow>(WaveRowName, TEXT("StartWeaveSpawning"));
	if (SpawnEnemyDataRow)
	{
		CachedSpawnEnemyDatas = SpawnEnemyDataRow->Enemies;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[AEnemySpawnDirector] Wave(%s)에 대한 스폰 데이터가 없습니다."), *WaveRowName.ToString());
		return;
	}


	for (const FEnemySpawnEntry& EnemyData : CachedSpawnEnemyDatas)
	{
		FEnemyDataRow* EnemyDataRow = EnemyDataTable->FindRow<FEnemyDataRow>(EnemyData.EnemyRowName, TEXT("SetSpawnEnemies"));
		if (EnemyDataRow)
		{
			CachedEnemyRowDatas.Add(EnemyDataRow);
			UE_LOG(LogTemp, Warning, TEXT("[AEnemySpawnDirector] Wave(%s) %d에 스폰할 적: %s"), *WaveRowName.ToString(), WaveNumber, *EnemyDataRow->EnemyName.ToString());

			// 여기서 해당 웨이브에 필요한 적들을 모두 비동기 로드
			UAssetManager& AssetManager = UAssetManager::Get();

			AssetManager.LoadPrimaryAsset(
				EnemyDataRow->EnemyPrimaryDataAsset, 
				{}, 
				FStreamableDelegate::CreateUObject(this, &AEnemySpawnDirector::OnEnemyAssetLoaded), 
				FStreamableManager::DefaultAsyncLoadPriority
			);

		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[AEnemySpawnDirector] Wave(%s) %d에 스폰할 적 데이터가 없습니다: %s"), *WaveRowName.ToString(), WaveNumber, *EnemyData.EnemyRowName.ToString());
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

	UWorld* World = GetWorld();
	check(World);
	UObjectPoolSubsystem* Pool = World->GetSubsystem<UObjectPoolSubsystem>();
	check(Pool);
	Pool->InitializePoolForClass(EnemyClass, 32);


}

void AEnemySpawnDirector::OnEnemyAssetLoaded()
{
}


