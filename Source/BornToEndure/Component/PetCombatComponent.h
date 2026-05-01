#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Delegates/Delegate.h"
#include "Data/PetProjectileItemDataAsset.h"
#include "Stat/PetStatTypes.h"
#include "GameplayTagContainer.h"
#include "PetCombatComponent.generated.h"

class UArrowComponent;
class ABaseProjectile;
class IPetStatProviderInterface;
class IPetItemProviderInterface;

/**
 * @brief 공격 정보를 담는 구조체
 * @details
 * - 공격 데미지, 크리티컬 여부, 속성 태그, 발사체 구성 등 공격 실행에 필요한 모든 정보를 포함
 * - 매 공격 시 스탯과 아이템 정보를 조합하여 BuildAttackInfo()에서 생성되며, 최종적으로 발사체에 전달됨
 * - 생성된 후 버려지기에 스탯이 중간에 변경되어서 이미 공격된 공격은 영향을 받지 않음
 */
USTRUCT(BlueprintType)
struct FPetAttackInfo
{
	GENERATED_BODY()

	// 최종 데미지, 크리티컬 여부, 크리티컬 배율
	float FinalDamage = 0.f;
	bool bIsCritical = false;
	float CriticalMultiplier = 1.5f;

	// 속성
	FGameplayTag ElementTag;

	// 발사체 구성
	TSubclassOf<AActor> ProjectileClass;
	int32 ProjectileCount = 1;
	float ProjectileSpeed = 1200.f;
	float ProjectileSize = 1.f;
	EProjectilePattern Pattern = EProjectilePattern::Single;
};


/**
 * @brief Pet의 전투(공격)를 담당하는 컴포넌트
 * @details
 * - StatProvider, ItemProvider 인터페이스로만 통신하며 외부 컴포넌트를 직접 참조하지 않음
 * - 공격 타이머를 관리하며 EndPlay에서 반드시 정리하여 댕글링 참조 크래시 방지
 */
UCLASS(ClassGroup = (Pet), meta = (BlueprintSpawnableComponent))
class BORNTOENDURE_API UPetCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPetCombatComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:

	/** * @brief Delegate 시그니처 선언으로, 공격 시 사운드와 나이아가라 스폰 호출 */
	DECLARE_DELEGATE_TwoParams(FSpawnSoundAtLocation, FName, FVector);
	FSpawnSoundAtLocation SoundDelegate;

	DECLARE_DELEGATE_TwoParams(FSpawnNiagaraAtLocation, FName, FVector);
	FSpawnNiagaraAtLocation NiagaraDelegate;

	/** * @brief UProjectilePoolSubsystem에서 사용할 풀 크기를 설정하는 변수 */
	UPROPERTY(EditAnywhere, Category = "Projectile")
	int32 ProjectilePoolSize;

	/** * @brief 사운드와 나이아가라 애셋을 참조하는 변수 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Effect|Sound", meta = (AllowedTypes = "SoundDataAsset"))
	FPrimaryAssetId AttackSoundId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Effect|Niagara", meta = (AllowedTypes = "NiagaraDataAsset"))
	FPrimaryAssetId AttackNiagaraId;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<AActor> DefaultProjectileClass;

	/**
	 * @brief StatProvider와 ItemProvider를 설정하는 함수
	 * @details
	 * - StatComponent와 ItemComponent를 직접 참조하지 않고 Interface로 통신하기 위함
	 * - Setter 함수로 UPetCompanionCharacter::BeginPlay에서 호출
	 * - UInterface를 UPROPERTY로 저장하려면 TScriptInterface가 필요
	 * @param InStatProvider StatProvider 인터페이스 참조
	 * @param InItemProvider ItemProvider 인터페이스 참조
	 */
	void SetProviders(TScriptInterface<IPetStatProviderInterface> InStatProvider,
					  TScriptInterface<IPetItemProviderInterface> InItemProvider);


	void StartAttack();
	void StopAttack();
	void OnAttack(const FVector& TargetVector);

	/**
	 * @brief 공격 속도 변경 시 Timer 반복 시간을 갱신한다
	 * @details 
	 * - PetStatComponent::OnStatChanged 델리게이트에 바인딩한다
	 * - PetCompanionCharacter::BeginPlay에서 바인딩한다
	 */
	void OnStatChanged(EPetStatType StatType, float NewValue);

private:

	void OnAttackSound(const FVector& SpawnLocation) const;
	void OnAttackNiagara(const FVector& SpawnLocation) const;

	/** * @brief 발사체 풀을 초기화하는 함수 */
	void InitializeProjectilePool();

	UPROPERTY()
	TScriptInterface<IPetStatProviderInterface> StatProvider;

	UPROPERTY()
	TScriptInterface<IPetItemProviderInterface> ItemProvider;

	FTimerHandle AttackTimerHandle;
	bool bIsAttacking = false;
	FVector CurrentTargetLocation = FVector::ZeroVector;

	/**
	 * @brief 공격 속도에 따라 Timer에 의해 반복적으로 실행되는 함수
	 * @details
	 * - Target이 지정되면 StartAttack -> RefreshAttackTimer로 호출되며 공격 속도에 따라 ExecuteAttack 함수 반복 실행을 설정
	 * - BuildAttackInfo에서 공격 정보를 가져온 후 SpawnProjectiles로 넘겨주며 공격을 실행
	 */
	void ExecuteAttack();
	void RefreshAttackTimer();

	/**
	 * @brief 모든 스탯과 아이템을 조합하여 최종 공격 정보를 가진 FPetAttackInfo 구조체를 생성함
	 * @return 최종 공격 정보가 담긴 FPetAttackInfo 구조체
	 */
	FPetAttackInfo BuildAttackInfo() const;

	/**
	 * @brief 공격 정보와 위치를 바탕으로 발사체를 스폰하는 함수
	 * @param AttackInfo 최종 공격 정보가 담긴 FPetAttackInfo 구조체로 이를 바탕으로 공격 액터를 생성 및 스폰
	 * @param TargetLocation 공격의 최종 목적지 위치
	 */
	void SpawnProjectiles(const FPetAttackInfo& AttackInfo, const FVector& TargetLocation);

	/**
	 * @brief 발사체 패턴에 따른 방향 벡터 배열을 계산한다.
	 * - 단발, 점사, 나선 등 패턴에 따른 각 발사체가 나아갈 방향을 의미함
	 */
	TArray<FVector> CalculateProjectileDirections(
		EProjectilePattern Pattern,
		int32 Count,
		const FVector& Origin,
		const FVector& TargetLocation) const;


public:

	void SetAttackTarget(AActor* NewTarget);
	void ClearAttackTarget();
	AActor* GetCurrentTarget() const { return CurrentTarget.Get(); }

private:

	TWeakObjectPtr<AActor> CurrentTarget;

};
