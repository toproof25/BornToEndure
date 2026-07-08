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

	/*
	// 어떤 이유인지는 모르겠지만 적 공격하면서 변경된? 거 때문에 적 탐지가 안되는 오류.
    Params.AddIgnoredActor(Owner);

    const bool bHit = GetWorld()->OverlapMultiByChannel(
        OverlapResults,
        Owner->GetActorLocation(),
        FQuat::Identity,
        EnemyChannel,
        Sphere,
        Params
    );
	*/

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(EnemyChannel); // 예: Enemy Object Channel

	const bool bHit = GetWorld()->OverlapMultiByObjectType(
		OverlapResults,
		Owner->GetActorLocation(),
		FQuat::Identity,
		ObjectParams,
		Sphere,
		Params
	);

	if (!bHit)
	{
		UE_LOG(LogTemp, Log, TEXT("[EnemyDetectorComponent]: No enemies detected."));
		return;
	}

    for (const FOverlapResult& Result : OverlapResults)
    {
        ABaseEnemyCharacter* Enemy = Cast<ABaseEnemyCharacter>(Result.GetActor());
        // IsValid + IsPendingKillPending 동시 체크
        if (IsValid(Enemy))
        {
            CachedEnemies.Add(Enemy);
        }
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[EnemyDetectorComponent]: Invalid enemy detected: %s"), *GetNameSafe(Result.GetActor()));
		}
    }

	UE_LOG(LogTemp, Log, TEXT("[EnemyDetectorComponent]: Detected %d enemies."), CachedEnemies.Num());
}
