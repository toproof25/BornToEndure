#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Delegates/Delegate.h"
#include "PetCombatComponent.generated.h"


class UArrowComponent;
class ABaseProjectile;


// Delegate 선언
DECLARE_DELEGATE_TwoParams(FSpawnSoundAtLocation, FName, FVector);
DECLARE_DELEGATE_TwoParams(FSpawnNiagaraAtLocation, FName, FVector);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
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
	FSpawnSoundAtLocation SoundDelegate;
	FSpawnNiagaraAtLocation NiagaraDelegate;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	//TObjectPtr<UArrowComponent> ProjectilePoint;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<ABaseProjectile> ProjectileClass;

	/** * @brief UProjectilePoolSubsystem에서 사용할 풀 크기를 설정하는 변수 */
	UPROPERTY(EditAnywhere, Category = "Projectile")
	int32 ProjectilePoolSize;

	/** * @brief 사운드와 나이아가라 애셋을 참조하는 변수 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Effect|Sound", meta = (AllowedTypes = "SoundDataAsset"))
	FPrimaryAssetId AttackSoundId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Effect|Niagara", meta = (AllowedTypes = "NiagaraDataAsset"))
	FPrimaryAssetId AttackNiagaraId;


	UFUNCTION()
	void OnAttack(const FVector& TargetVector);


private:

	void OnAttackSound(const FVector& SpawnLocation) const;
	void OnAttackNiagara(const FVector& SpawnLocation) const;

	/** * @brief발사체 풀을 초기화하는 함수 */
	void InitializeProjectilePool();

};
