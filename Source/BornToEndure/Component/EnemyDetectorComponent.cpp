#include "Component/EnemyDetectorComponent.h"
#include "Character/Enemy/BaseEnemyCharacter.h"
#include "Engine/World.h"
#include "TimerManager.h"

UEnemyDetectorComponent::UEnemyDetectorComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UEnemyDetectorComponent::BeginPlay()
{
    Super::BeginPlay();

    GetWorld()->GetTimerManager().SetTimer(
        UpdateTimerHandle,
        this,
        &UEnemyDetectorComponent::UpdateEnemyList,
        UpdateInterval,
        true,
        0.0f  // 즉시 첫 실행
    );
}

void UEnemyDetectorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
}

void UEnemyDetectorComponent::UpdateEnemyList()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    OverlapResults.Reset();
    CachedEnemies.Reset();

    FCollisionShape Sphere = FCollisionShape::MakeSphere(DetectRadius);
    FCollisionQueryParams Params(SCENE_QUERY_STAT(EnemyDetectorOverlap), false);
    Params.AddIgnoredActor(Owner);

    const bool bHit = GetWorld()->OverlapMultiByChannel(
        OverlapResults,
        Owner->GetActorLocation(),
        FQuat::Identity,
        EnemyChannel,
        Sphere,
        Params
    );

    if (!bHit) return;

    for (const FOverlapResult& Result : OverlapResults)
    {
        ABaseEnemyCharacter* Enemy = Cast<ABaseEnemyCharacter>(Result.GetActor());
        // IsValid + IsPendingKillPending 동시 체크
        if (IsValid(Enemy))
        {
            CachedEnemies.Add(Enemy);
        }
    }
}