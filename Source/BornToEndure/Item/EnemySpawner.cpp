#include "Item/EnemySpawner.h"
#include "Subsystem/ObjectPoolSubsystem.h"
#include "NavigationSystem.h"
#include "Character/Enemy/BaseEnemyCharacter.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Component/PlayerExperienceComponent.h"
#include "Character/Player/PlayerCharacter.h"

AEnemySpawner::AEnemySpawner()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AEnemySpawner::BeginPlay()
{
    Super::BeginPlay();

    UWorld* World = GetWorld();
    check(World);
    UObjectPoolSubsystem* Pool = World->GetSubsystem<UObjectPoolSubsystem>();
    check(Pool);

    // 풀 초기화
    if (EnemyClass)
    {
        Pool->InitializePoolForClass(EnemyClass, PoolSize);
    }

    // 스폰 시작
    World->GetTimerManager().SetTimer(
        SpawnTimerHandle,
        this,
        &AEnemySpawner::SpawnEnemy,
        SpawnInterval,
        true
    );

}

void AEnemySpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Pool에서 적 제거
    if (UWorld* World = GetWorld())
    {
        if (UObjectPoolSubsystem* Pool = World->GetSubsystem<UObjectPoolSubsystem>())
        {
			Pool->RemovePoolActor(EnemyClass);
        }
    }

    // 타이머 정리
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(SpawnTimerHandle);
    }

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
		BaseEnemy->SetExpReward(999.99f);
		BaseEnemy->SetGoldReward(9999);

        // 적을 스폰할 때 마다 플레이어를 찾은 후 Delegate 연결하기 (일단 테스트)
        // 적마다 개별적인 Delegate를 직접 Player와 연결
        APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
        UPlayerExperienceComponent* PlayerXP = PlayerChar ? PlayerChar->FindComponentByClass<UPlayerExperienceComponent>() : nullptr;
        if (PlayerXP)
        {
            PlayerXP->RegisterEnemyPayload(BaseEnemy);
            UE_LOG(LogTemp, Log, TEXT("Registering enemy %s with player XP component"), *BaseEnemy->GetName());
        }
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
    const bool bFound = NavSys->GetRandomReachablePointInRadius(
        GetActorLocation(), SpawnRadius, NavLocation);

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