#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/OverlapResult.h"
#include "EnemyDetectorComponent.generated.h"

class ABaseEnemyCharacter;

/**
 * Player에 부착되어 주기적으로 주변 적을 탐색한다.
 * 모든 Pet이 개별 OverlapMulti를 호출하는 대신 이 컴포넌트의 캐시를 공유한다.
 * Pet N마리 -> N번 Overlap 비용 -> 단 1번 Overlap 비용
 */


/**
 * @brief Player에 부착되어 주기적으로 OverlapMultiByChannel으로 주변 적을 탐색하며 TArray에 저장하는 컴포넌트
 * - 모든 Pet이 개별 OverlapMulti를 호출하는 대신 이 컴포넌트의 캐시를 공유하여 Pet이 탐색된 적 TArray를 가져가도록 함
 * - Pet N마리 -> N번 Overlap 비용 -> 단 1번 Overlap 비용으로 최적화할 수 있음 
 */
UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class BORNTOENDURE_API UEnemyDetectorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnemyDetectorComponent();

    /** Pet들이 호출하여 캐시된 적 목록을 가져간다 */

    /**
     * @brief Pet들이 호출하여 캐시된 적 목록을 가져가기 위한 함수
     * @return 주기적으로 탐색된 적 TArray를 const 참조로 반환
     */
    const TArray<TWeakObjectPtr<ABaseEnemyCharacter>>& GetCachedEnemies() const { return CachedEnemies; }

    /** * @brief Player 위치 기준으로 탐색 반경 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detection")
    float DetectRadius = 2000.0f;

    /** * @brief 갱신 주기로 매 프레임보다 낮게 유지할 수 있도록하며 Timer로 관리됨*/
    UPROPERTY(EditAnywhere, Category = "Detection")
    float UpdateInterval = 0.15f;

    UPROPERTY(EditAnywhere, Category = "Detection")
    TEnumAsByte<ECollisionChannel> EnemyChannel = ECC_GameTraceChannel3;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    void UpdateEnemyList();

    TArray<TWeakObjectPtr<ABaseEnemyCharacter>> CachedEnemies;
    TArray<FOverlapResult> OverlapResults;
    FTimerHandle UpdateTimerHandle;
};