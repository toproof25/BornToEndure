/**
 * @file EnemySpawner.h
 * @brief 적 스포너 클래스 정의
 * @author toproof (kmnlmn123@gmail.com)
 * @date 2026-04-27
 * @details
 * - 에디터에서 설정한 EnemyClass를 일정 간격으로 스폰하고 풀링 시스템과 연동
 */


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"


UCLASS()
class BORNTOENDURE_API AEnemySpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	AEnemySpawner();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private: 
    /** @brief 스폰할 적 클래스. 블루프린트에서 설정 */
    UPROPERTY(EditAnywhere, Category = "Spawner")
    TSubclassOf<AActor> EnemyClass;

    /** @brief 스폰 반경 (cm). 스포너 위치 중심 */
    UPROPERTY(EditAnywhere, Category = "Spawner", meta = (ClampMin = "100.0"))
    float SpawnRadius = 1500.f;

    /** @brief 스폰 간격 (초) */
    UPROPERTY(EditAnywhere, Category = "Spawner", meta = (ClampMin = "0.1"))
    float SpawnInterval = 2.f;

    /** @brief 한 번에 스폰할 최대 활성 적 수 */
    UPROPERTY(EditAnywhere, Category = "Spawner")
    int32 MaxActiveEnemies = 30;

    /** @brief 초기 풀 크기 */
    UPROPERTY(EditAnywhere, Category = "Spawner")
    int32 PoolSize = 50;

    FTimerHandle SpawnTimerHandle;

    void SpawnEnemy();
    bool GetRandomSpawnLocation(FVector& OutLocation) const;

    /** 현재 활성화된 적 수 추적 (풀 통계 기반) */
    int32 GetActiveEnemyCount() const;
};