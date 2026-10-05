/**
* @file EnemySpawnDirector.h
* @brief 적 스포너를 관리하는 클래스
*/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Data/DataTableRow/EnemyDataRow.h"
#include "Data/DataTableRow/SpawnEnemyDataRow.h"

#include "EnemySpawnDirector.generated.h"

class AEnemySpawner;
class UDataTable;


UCLASS()
class BORNTOENDURE_API AEnemySpawnDirector : public AActor
{
	GENERATED_BODY()
	
public:	
	AEnemySpawnDirector();

	void StartWeaveSpawning();
	void StopWeaveSpawning();

	void SetSpawnEnemies(int32 WaveNumber);

	void KillAllEnemies();

protected:
	virtual void BeginPlay() override;

private:

	UPROPERTY(EditAnywhere)
	TObjectPtr<UDataTable> SpawnEnemyDataTable;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UDataTable> EnemyDataTable;

	/** @brief 스폰할 적 클래스. 블루프린트에서 설정 */
	UPROPERTY(EditAnywhere, Category = "Enemy", meta=(AllowPrivateAccess = "true"))
	TSubclassOf<AActor> EnemyClass;

	/**
	 * @brief 스폰할 적을 찾기 위한 RowName을 담아두는 변수
	 * - 이 배열을 바탕으로 DT_EnemyDataTable에서 Row 구조체를 가져온다.
	 */
	TArray<FEnemySpawnEntry> CachedSpawnEnemyDatas;

	/**
	 * @brief 실제 스포너에게 전달하기 위한 Row 구조체를 담아두는 변수
	 * - 스포너에게 Row 구조체를 전달하여, 스포너는 해당 구조체를 바탕으로 적을 스폰한다
	 */
	TArray<FEnemyDataRow*> CachedEnemyRowDatas;

	TArray<TObjectPtr<AEnemySpawner>> EnemySpawnPoints;


	void OnEnemyAssetLoaded();

};
