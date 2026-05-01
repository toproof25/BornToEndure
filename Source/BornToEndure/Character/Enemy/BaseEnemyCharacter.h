#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/Poolable.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Delegates/Delegate.h"
#include "Data/GameTypes.h"

#include "BaseEnemyCharacter.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBaseEnemyCharacter, Log, All);


// 적이 비활성화 될 때 호출 (아직 사용 구독 x)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyDeactivated, ABaseEnemyCharacter*, Enemy);

// Niagara, Sound 재생 Delegate
DECLARE_DELEGATE_TwoParams(FOnEnemyHitSound, FName, FVector);
DECLARE_DELEGATE_TwoParams(FOnEnemyHitNiagara, FName, FVector);

UCLASS()
class BORNTOENDURE_API ABaseEnemyCharacter : public ACharacter, public IPoolable
{
    GENERATED_BODY()

public:
    ABaseEnemyCharacter();
    
    /**
     * @brief Pool System에서 활성화, 비활성화 호출 함수로 IPoolable Interface에서 선언되며, 오버라이드하여 사용
     */
    virtual void ActivateActor_Implementation() override;
    virtual void DeactivateActor_Implementation() override;

    virtual float TakeDamage(
        float DamageAmount,
        struct FDamageEvent const& DamageEvent,
        AController* EventInstigator,
        AActor* DamageCauser) override;

    void SetOwningSpawner(AActor* Spawner) { OwningSpawner = Spawner; }

    UPROPERTY(EditAnywhere, Category = "AI")
    TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

    /**
     * @brief 적이 제거될 때 방송하는 Delegate로 아래 Delegate 사용중 (추후 제거하거나 재사용 검토)
     */
    UPROPERTY(BlueprintAssignable, Category = "Combat")
    FOnEnemyDeactivated OnEnemyDeactivated;

    /**
     * @brief 적 사망 시 방송하는 Delegate로 경험치, 보상에 대한 FEnemyRewardPayload 구조체를 전달한다
     * @note Player가 같은 Delegate를 구독한다
     */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnEnemyKilled OnEnemyKilled;

	/**
	 * @brief 적 사망 시 전달할 보상&킬데이터 정보 구조체로 GameTypes.h에서 정의된다
	 * - 경험치, 골드 보상 (스포너에서 정의)
	 * - Pet별 가한 데미지 정보 (킬에 기여한 펫이 있을 경우)
	 */
	FEnemyRewardPayload EnemyRewardPayload;
	void SetExpReward(float Exp) { EnemyRewardPayload.ExpReward = Exp; }
	void SetGoldReward(int32 Gold) { EnemyRewardPayload.GoldReward = Gold; }
    void ResetRewardPayload() { EnemyRewardPayload.Reset(); }

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float MaxHealth = 100.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
    float CurrentHealth = 100.f;

    UPROPERTY(EditAnywhere, Category = "Stats")
    float MoveSpeed = 300.f;

    UPROPERTY(EditAnywhere, Category = "Effects")
    FPrimaryAssetId HitEnemySoundId;

    UPROPERTY(EditAnywhere, Category = "Effects")
    FPrimaryAssetId HitEnemyNiagaraId;

private:
    void HandleDeath();

    TWeakObjectPtr<AActor> OwningSpawner;
    FOnEnemyHitSound OnEnemyHitSound;
    FOnEnemyHitNiagara OnEnemyHitNiagara;
};