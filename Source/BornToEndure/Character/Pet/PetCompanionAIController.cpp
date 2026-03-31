#include "Character/Pet/PetCompanionAIController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"

APetCompanionAIController::APetCompanionAIController()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APetCompanionAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void APetCompanionAIController::SetPlayerTarget(APawn* NewOwnerPawn)
{
	if (NewOwnerPawn)
	{
		PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		GetWorldTimerManager().SetTimer(MoveTimerHandle, this, &APetCompanionAIController::UpdateRandomMoveLocation, 3.0f, true);
		UE_LOG(LogTemp, Log, TEXT("Player를 인식했습니다: %s"), *PlayerPawn->GetName());
	}
}

void APetCompanionAIController::UpdateRandomMoveLocation()
{
    if (!PlayerPawn || !GetPawn()) return;

    FVector PlayerLoc = PlayerPawn->GetActorLocation();
    FVector PetLoc = GetPawn()->GetActorLocation();

    // 1. 거리 체크 (DistSquared 활용 최적화)
    float DistanceSquaredToPlayer = FVector::DistSquared(PlayerLoc, PetLoc);

    if (DistanceSquaredToPlayer > ReturnRadiusSquared)
    {
        // 너무 멀면 즉시 플레이어 위치로 복귀 시도
        MoveToLocation(PlayerLoc, 50.0f);
        return;
    }

    // 2. 플레이어 주변의 랜덤한 위치 계산
    // 팁: 단순히 랜덤 좌표를 찍는 것보다 내비게이션 메시(NavMesh) 위의 유효한 지점을 찾는 것이 훨씬 안전합니다.
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (NavSys)
    {
        FNavLocation RandomNavLocation;

        // 플레이어 위치를 기준으로 MaxRadius 반경 내의 무작위 이동 가능 지점을 찾습니다.
        bool bFound = NavSys->GetRandomReachablePointInRadius(PlayerLoc, MaxRadius, RandomNavLocation);

        if (bFound)
        {
            // 너무 플레이어와 겹치지 않도록 최소 거리(MinRadius) 체크를 추가할 수 있습니다.
            float DistSqToNewPoint = FVector::DistSquared(PlayerLoc, RandomNavLocation.Location);
            if (DistSqToNewPoint < (MinRadius * MinRadius))
            {
                // 너무 가까우면 이번 틱은 쉬거나 다시 계산 (간단하게 플레이어 위치에서 약간 오프셋을 줌)
                RandomNavLocation.Location += (RandomNavLocation.Location - PlayerLoc).GetSafeNormal() * MinRadius;
            }

            // 3. 해당 위치로 이동 명령 (부드러운 가속/감속 포함)
            MoveToLocation(RandomNavLocation.Location, 50.0f, true, true, true, true);
        }
    }
}