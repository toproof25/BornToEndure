/**
* @file SpawnEnemyDataRow.h
* @brief DT_SpawnEnemyDataTable의 행을 구성하는 DaraRow
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SpawnEnemyDataRow.generated.h"

USTRUCT(BlueprintType)
struct FEnemySpawnEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FName EnemyRowName;

	UPROPERTY(EditAnywhere)
	float Weight = 1.0f;
};

USTRUCT(BlueprintType)
struct FSpawnEnemyDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float WaveDuration = 60.0f;

	UPROPERTY(EditAnywhere)
	float SpawnInterval = 1.0f;

	UPROPERTY(EditAnywhere)
	int32 MaxAliveEnemies = 50;

	UPROPERTY(EditAnywhere)
	TArray<FEnemySpawnEntry> Enemies;

};

