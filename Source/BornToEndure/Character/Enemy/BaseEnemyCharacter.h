/**
* @file BaseEnemyCharacter.h
* @brief 적 캐릭터의 기본 클래스
* 
* - Poolable 인터페이스 기반 활성화/비활성화
* - DataTable 기반 스탯 초기화
* - 공격 범위 Overlap 감지
* - 피격/사망 처리 및 보상 Delegate 방송
*/

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/Poolable.h"
#include "Data/GameTypes.h"
#include "Data/DataTableRow/EnemyDataRow.h"

#include "BaseEnemyCharacter.generated.h"

class UBoxComponent;
class APlayerCharacter;
class UBehaviorTree;
class ABaseEnemyCharacter;
class UPrimitiveComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogBaseEnemyCharacter, Log, All);

/** 적이 비활성화 될 때 호출 (아직 사용 구독 x) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyDeactivated, ABaseEnemyCharacter*, Enemy);

/** Niagara, Sound 재생 Delegate */
DECLARE_DELEGATE_TwoParams(FOnEnemyHitSound, FName, FVector);
DECLARE_DELEGATE_TwoParams(FOnEnemyHitNiagara, FName, FVector);

UCLASS()
class BORNTOENDURE_API ABaseEnemyCharacter : public ACharacter, public IPoolable
{
    GENERATED_BODY()

public:
    ABaseEnemyCharacter();

	/**
	 * @brief EnemyDataTable에서 가져온 데이터를 기반으로 적 초기화
	 * @param EnemyData EnemyDataRow 구조체로 DataTable에서 가져온 적 데이터
	 */
	void InitializeEnemy(const FEnemyDataRow& EnemyData);
    
    /**
     * @brief Pool System에서 활성화, 비활성화 호출 함수로 IPoolable Interface에서 선언되며, 오버라이드하여 사용
     */
    virtual void ActivateActor_Implementation() override;
    virtual void DeactivateActor_Implementation() override;

    virtual float TakeDamage(
        float DamageAmount,
        struct FDamageEvent const& DamageEvent,
        AController* EventInstigator,
        AActor* DamageCauser
    ) override;


    void SetOwningSpawner(AActor* Spawner) { OwningSpawner = Spawner; }

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
    void ResetRewardPayload() { EnemyRewardPayload.Reset(); }

	UBehaviorTree* GetEnemyBehaviorTree() {	return BehaviorTreeAsset; }


protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnAttackBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnAttackEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> AttackRangeCollision;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|AI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

    TWeakObjectPtr<AActor> OwningSpawner;
    FOnEnemyHitSound OnEnemyHitSound;
    FOnEnemyHitNiagara OnEnemyHitNiagara;

    FTimerHandle AttackTimerHandle;
	bool bCanAttack = true;
	float AttackCooldown = 3.0f; 
	TWeakObjectPtr<APlayerCharacter> TargetPlayerCharacter = nullptr;

	void DelegateBindng();
	void DelegateUnbinding();
	void HandleDeath();

    /**
     * @brief 플레이어에게 공격을 수행하는 함수
	 * @note 
	 * - AttackRangeCollision의 OnComponentBeginOverlap 이벤트에서 호출되며, 공격 쿨타임이 끝나면 공격을 수행한다.
	 * - 공격의 경우 ApplayDamage를 통해 플레이어에게 데미지 준다
     */
    void AttackPlayer();

    /**
	 * @brief 공격 상태를 초기화 하는 함수로 AttackPlayer()에서 공격 후 호출 AttackCooldown 시간 후에 bCanAttack을 true로 설정한다
     */
    void ResetAttack();

};
