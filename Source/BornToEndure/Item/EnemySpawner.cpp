#include "Item/EnemySpawner.h"
#include "Subsystem/ObjectPoolSubsystem.h"
#include "NavigationSystem.h"
#include "Character/Enemy/BaseEnemyCharacter.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Component/PlayerExperienceComponent.h"
#include "Character/Player/PlayerCharacter.h"
#include "Engine/DataTable.h"
#include "Data/DataTableRow/EnemyDataRow.h"
#include "PlayerState/CombatPlayerState.h"

AEnemySpawner::AEnemySpawner()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AEnemySpawner::StartWeaveSpawning(const FEnemyDataRow* EnemyDataRow)
{
	CachedEnemyDataRow = EnemyDataRow;

	UWorld* World = GetWorld();
	if (!World) return;

	// 스폰 시작
	World->GetTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&AEnemySpawner::SpawnEnemy,
		SpawnInterval,
		true
	);
}

void AEnemySpawner::StopWeaveSpawning()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// 스폰 중지
	World->GetTimerManager().ClearTimer(SpawnTimerHandle);
}

void AEnemySpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
    Super::EndPlay(EndPlayReason);
}

void AEnemySpawner::SpawnEnemy()
{
    // 최대 적 수 초과 시 스킵
    if (GetActiveEnemyCount() >= MaxActiveEnemies) return;

    UWorld* World = GetWorld();
    if (!World) return;
    UObjectPoolSubsystem* Pool = World->GetSubsystem<UObjectPoolSubsystem>();
    if (!Pool) return;

    FVector SpawnLocation;
    if (!GetRandomSpawnLocation(SpawnLocation)) return;

    AActor* Enemy = Pool->RequestPoolActor(EnemyClass);
    if (!Enemy) return;

	// BaseEnemy에 스포너 참조 전달 및 보상 세팅 (테스트용으로 고정값)
    if (ABaseEnemyCharacter* BaseEnemy = Cast<ABaseEnemyCharacter>(Enemy))
    {
        BaseEnemy->SetOwningSpawner(this);

        // 적을 스폰할 때 마다 플레이어를 찾은 후 Delegate 연결하기 (일단 테스트)
        // 적마다 개별적인 Delegate를 직접 PlayerState와 연결
        APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0); if (!PC) return;
        ACombatPlayerState* PS = PC->GetPlayerState<ACombatPlayerState>(); if (!PS) return;
        UPlayerExperienceComponent* PlayerXP = PS->FindComponentByClass<UPlayerExperienceComponent>();
        if (PlayerXP) PlayerXP->RegisterEnemyPayload(BaseEnemy);

		// DataTable에서 가져온 적에 대한 보상, 스탯 설정을 BaseEnemyCharacter로 전달하여 초기화하는 로직도 추가 예정
		BaseEnemy->InitializeEnemy(*CachedEnemyDataRow);

		/*
        const FEnemyDataRow* EnemyData = CachedEnemyDataMap.Find(FName("Test_1"));
        if (EnemyData)
        {
			BaseEnemy->InitializeEnemy(*CachedEnemyDataRow);
			UE_LOG(LogTemp, Log, TEXT("Spawn Enemy: Initializing enemy %s with data from DataTable"), *BaseEnemy->GetName());
        }
        else
        {
			UE_LOG(LogTemp, Warning, TEXT("Spawn Enemy: No data found for enemy %s in CachedEnemyDataMap"), *BaseEnemy->GetName());
        }
		*/
    }

    // 위치 회전 초기화
    Enemy->SetActorLocationAndRotation(
        SpawnLocation,
        FRotator::ZeroRotator,
        false, nullptr,
        ETeleportType::TeleportPhysics
    );


}

bool AEnemySpawner::GetRandomSpawnLocation(FVector& OutLocation) const
{
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys) return false;

    FNavLocation NavLocation;
    const bool bFound = NavSys->GetRandomReachablePointInRadius(GetActorLocation(), SpawnRadius, NavLocation);

    if (bFound)
    {
        OutLocation = NavLocation.Location;
    }
    return bFound;
}

int32 AEnemySpawner::GetActiveEnemyCount() const
{
    UWorld* World = GetWorld();
    if (!World) return 0;
    UObjectPoolSubsystem* Pool = World->GetSubsystem<UObjectPoolSubsystem>();
    if (!Pool) return 0;

    int32 Total, Active, Inactive;
    Pool->GetPoolStats(EnemyClass, Total, Active, Inactive);
    return Active;
}
