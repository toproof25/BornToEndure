#include "ObjectPoolSubsystem.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Poolable.h"

DEFINE_LOG_CATEGORY(LogObjectPoolSubsystem);

UWorld* UObjectPoolSubsystem::GetWorldChecked()
{
    UWorld* World = GetWorld();
    check(World);
    return World;
}

void UObjectPoolSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UObjectPoolSubsystem::InitializePoolForClass(TSubclassOf<AActor> ActorClass, int32 PoolSize)
{
    if (!ActorClass) return;

    UWorld* World = GetWorldChecked();


    // AActor를 부모 클래스로 두기에 각 자식 클래스별로 관리할 수 있도록 사용
    UClass* ClassKey = ActorClass.Get();

    // 이미 존재하면 무시
    if (ActorPools.Contains(ClassKey)) return;

    PoolSizes.Add(ClassKey, PoolSize);
    TArray<AActor*>& Pool = ActorPools.FindOrAdd(ClassKey);

    for (int32 i = 0; i < PoolSize; ++i)
    {
        // 발사체 스폰 후 비활성화
        AActor* PoolActor = World->SpawnActor<AActor>(ClassKey, FVector::ZeroVector, FRotator::ZeroRotator);
        if (PoolActor)
        {
            if (IPoolable* Poolable = Cast<IPoolable>(PoolActor))
            {
                Poolable->Execute_DeactivateActor(PoolActor);
                Pool.Add(PoolActor);
            }
        }
    }

    UE_LOG(LogObjectPoolSubsystem, Display, TEXT("Initialized UObjectPoolSubsystem for Class: %s with Size: %d"), *ClassKey->GetName(), PoolSize);
}

AActor* UObjectPoolSubsystem::RequestPoolActor(TSubclassOf<AActor> ActorClass)
{
    if (ActorClass == nullptr)
        return nullptr;


    UClass* ClassKey = ActorClass.Get();

    // 요청한 발사체가 풀에 존재하는지 확인 (PoolPtr은 배열의 시작 포인터가됨)
    TArray<AActor*>* PoolPtr = ActorPools.Find(ClassKey);
    if (!PoolPtr)
    {
        UE_LOG(LogObjectPoolSubsystem, Warning, TEXT("Requested UObjectPoolSubsystem Class not found in Pool: %s"), *ClassKey->GetName());
        return nullptr;
    }

    // 요청한 발사체 중 비활성화된 발사체를 활성화 하고 반환
    for (AActor* PoolActor : *PoolPtr)
    {
        if (PoolActor && PoolActor->IsHidden())
        {
            if (IPoolable* Poolable = Cast<IPoolable>(PoolActor))
            {
				Poolable->Execute_ActivateActor(PoolActor);
                UE_LOG(LogObjectPoolSubsystem, Display, TEXT("Requested UObjectPoolSubsystem from Pool: %s"), *ClassKey->GetName());
                return PoolActor;
            }
        }
    }

    // 발사체가 없으면 추가 생성 ( 스폰 -> 활성화 -> 바로 반환 )
    UWorld* World = GetWorldChecked();
    AActor* PoolActor = World->SpawnActor<AActor>(ClassKey, FVector::ZeroVector, FRotator::ZeroRotator);
    if (PoolActor)
    {
        PoolPtr->Add(PoolActor);

        // 풀 크기 갱신
        int32* SizePtr = PoolSizes.Find(ClassKey);
        if (SizePtr)
        {
            (*SizePtr)++;
        }
    }


    IPoolable* Poolable = Cast<IPoolable>(PoolActor);
    Poolable->Execute_ActivateActor(PoolActor);
    UE_LOG(LogObjectPoolSubsystem, Display, TEXT("No available UObjectPoolSubsystem, spawned new one for Class: %s"), *ClassKey->GetName());
    return PoolActor;
}

void UObjectPoolSubsystem::ReturnPoolActor(AActor* PoolActor)
{
    if (PoolActor)
    {
        if (IPoolable* Poolable = Cast<IPoolable>(PoolActor))
        {
            Poolable->Execute_DeactivateActor(PoolActor);
            UE_LOG(LogObjectPoolSubsystem, Display, TEXT("Returned UObjectPoolSubsystem to Pool: %s"), *PoolActor->GetName());
        }
    }
}

void UObjectPoolSubsystem::GetPoolStats(TSubclassOf<AActor> ActorClass, int32& OutTotal, int32& OutActive, int32& OutInactive) const
{
    OutTotal = 0;
    OutActive = 0;
    OutInactive = 0;

    if (!ActorClass) return;

    UClass* ClassKey = ActorClass.Get();

    if (const TArray<AActor*>* PoolPtr = ActorPools.Find(ClassKey))
    {
        OutTotal = PoolPtr->Num();

        for (AActor* Projectile : *PoolPtr)
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