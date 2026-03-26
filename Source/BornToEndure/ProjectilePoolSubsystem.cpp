#include "ProjectilePoolSubsystem.h"
#include "Engine/World.h"
#include "BaseProjectile.h"

DEFINE_LOG_CATEGORY(LogProjectilePoolSubsystem);

UWorld* UProjectilePoolSubsystem::GetWorldChecked()
{
    UWorld* World = GetWorld();
    check(World);
    return World;
}

void UProjectilePoolSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UProjectilePoolSubsystem::InitializePoolForClass(TSubclassOf<ABaseProjectile> ProjectileClass, int32 PoolSize)
{
    if (!ProjectileClass) return;

    UWorld* World = GetWorldChecked();


	// ABaseProjectile를 부모 클래스로 두기에 각 자식 클래스별로 관리할 수 있도록 사용
    UClass* ClassKey = ProjectileClass.Get();

    // 이미 존재하면 무시
    if (ProjectilePools.Contains(ClassKey)) return;

    PoolSizes.Add(ClassKey, PoolSize);
    TArray<ABaseProjectile*>& Pool = ProjectilePools.FindOrAdd(ClassKey);

    for (int32 i = 0; i < PoolSize; ++i)
    {
        // 발사체 스폰 후 비활성화
        ABaseProjectile* Projectile = World->SpawnActor<ABaseProjectile>(ClassKey, FVector::ZeroVector, FRotator::ZeroRotator);
        if (Projectile)
        {
            Projectile->SetActorHiddenInGame(true);
            Projectile->SetActorEnableCollision(false);
            Pool.Add(Projectile);
        }
    }

	UE_LOG(LogProjectilePoolSubsystem, Display, TEXT("Initialized Projectile Pool for Class: %s with Size: %d"), *ClassKey->GetName(), PoolSize);
}

ABaseProjectile* UProjectilePoolSubsystem::RequestProjectile(TSubclassOf<ABaseProjectile> ProjectileClass)
{
    if (ProjectileClass == nullptr)
        return nullptr;


    UClass* ClassKey = ProjectileClass.Get();

    // 요청한 발사체가 풀에 존재하는지 확인 (PoolPtr은 배열의 시작 포인터가됨)
    TArray<ABaseProjectile*>* PoolPtr = ProjectilePools.Find(ClassKey);
    if (!PoolPtr)
    {
		UE_LOG(LogProjectilePoolSubsystem, Warning, TEXT("Requested Projectile Class not found in Pool: %s"), *ClassKey->GetName());
        return nullptr;
    }

    // 요청한 발사체 중 비활성화된 발사체를 활성화 하고 반환
    for (ABaseProjectile* Projectile : *PoolPtr)
    {
        if (Projectile && Projectile->IsHidden())
        {
            Projectile->SetActorHiddenInGame(false);
            Projectile->SetActorEnableCollision(true);
			UE_LOG(LogProjectilePoolSubsystem, Display, TEXT("Requested Projectile from Pool: %s"), *ClassKey->GetName());
            return Projectile;
        }
    }

    // 발사체가 없으면 추가 생성 ( 스폰 -> 활성화 -> 바로 반환 )
    UWorld* World = GetWorldChecked();
    ABaseProjectile* Projectile = World->SpawnActor<ABaseProjectile>(ClassKey, FVector::ZeroVector, FRotator::ZeroRotator);
    if (Projectile)
    {
        Projectile->SetActorHiddenInGame(false);
        Projectile->SetActorEnableCollision(true);
        PoolPtr->Add(Projectile);

        // 풀 크기 갱신
        int32* SizePtr = PoolSizes.Find(ClassKey);
        if (SizePtr)
        {
            (*SizePtr)++;
        }
    }

	UE_LOG(LogProjectilePoolSubsystem, Display, TEXT("No available Projectile in Pool, spawned new one for Class: %s"), *ClassKey->GetName());
    return Projectile;
}

void UProjectilePoolSubsystem::ReturnProjectile(ABaseProjectile* Projectile)
{
    if (Projectile)
    {
		UE_LOG(LogProjectilePoolSubsystem, Display, TEXT("Returned Projectile to Pool: %s"), *Projectile->GetName());
        Projectile->SetActorHiddenInGame(true);
        Projectile->SetActorEnableCollision(false);
        // 위치 초기화 추가 예정
    }
}

void UProjectilePoolSubsystem::GetPoolStats(TSubclassOf<ABaseProjectile> ProjectileClass, int32& OutTotal, int32& OutActive, int32& OutInactive) const
{
    OutTotal = 0;
    OutActive = 0;
    OutInactive = 0;

    if (!ProjectileClass) return;

    UClass* ClassKey = ProjectileClass.Get();

    if (const TArray<ABaseProjectile*>* PoolPtr = ProjectilePools.Find(ClassKey))
    {
        OutTotal = PoolPtr->Num();

        for (ABaseProjectile* Projectile : *PoolPtr)
        {
            // IsHidden()이 true면 풀에서 대기 중, false면 월드에서 날아가고 있는 상태
            if (Projectile && Projectile->IsHidden())
            {
                OutInactive++;
            }
            else
            {
                OutActive++;
            }
        }
    }
}