/**
* @file BaseWeapon.h
* @brief Pet이 사용하는 무기 클래스의 최상위 클래스
*/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Data/CombatTypes.h"
#include "Data/DataTableRow/WeaponItemDataRow.h"

#include "UObject/PrimaryAssetId.h"

#include "BaseWeapon.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBaseWeapon, Log, All); // 로그 카테고리 선언

// Delegate 선언
DECLARE_DELEGATE_TwoParams(FSpawnSoundAtLocation, FName, FVector);
DECLARE_DELEGATE_TwoParams(FSpawnNiagaraAtLocation, FName, FVector);

class UStaticMeshComponent;
class UPetWeaponItemDataAsset;

UCLASS(Abstract)
class BORNTOENDURE_API ABaseWeapon : public AActor
{
	GENERATED_BODY()

public:
	ABaseWeapon();

	/**
	* @brief 무기 데이터 테이블 행 구조체를 설정하는 함수
	* - WeaponItemDataRow 구조체를 기반으로 무기 데이터를 설정
	* - 부모 클래스에서는 공통 데이터 설정만 수행하며, 자식 클래스에서 추가 데이터 설정 구현
	* - 항상 자식 클래스에서는 Super::SetWeaponData(ItemData)를 호출해야 함
	*/
	void InitializeWeapon(const UPetWeaponItemDataAsset& ItemData);

	/**
	* @brief 무기 공격 함수
	* - PetCombatComponent에서 호출되며 최종 공격 데이터를 받아 공격을 수행함
	* - 부모 클래스에서는 공격 데이터만 전달받고, 자식 클래스에서 실제 공격 로직 구현
	* - 순수 가상 함수(PURE_VIRTUAL)로 선언되어 자식 클래스에서 반드시 구현해야 함
	*/
	virtual void OnAttack(const FPetAttackInfo& AtkInfo) PURE_VIRTUAL(ABaseWeapon::OnAttack, );

protected:
	virtual void BeginPlay() override;
	virtual void OnInitalizeWeapon(const UPetWeaponItemDataAsset& ItemData) PURE_VIRTUAL(ABaseWeapon::OnInitalizeWeapon, );

	/**
	 * @brief 무기 메쉬를 참조하는 포인터
	 * @note 상호작용 시 해당 메쉬 획득 연결
	 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> WeaponStaticMesh;

	
	// ---------- 스폰, 공격 사운드, 나이아가라 설정 ----------

	/** * @brief Delegate 시그니처 선언으로, 공격 시 사운드와 나이아가라 스폰 호출 */
	FSpawnSoundAtLocation SoundDelegate;
	FSpawnNiagaraAtLocation NiagaraDelegate;

	/** * @brief 사운드와 나이아가라 애셋을 참조하는 변수 */
	FPrimaryAssetId SpawnSoundId;
	FPrimaryAssetId SpawnNiagaraId;
	FPrimaryAssetId AttackSoundId;
	FPrimaryAssetId AttackNiagaraId;
	
	/** * @brief 사운드와 나이아가라를 EffectSubsystem에 전달하여 호출*/
	void OnSpawnSoundAndNiagara(const FVector& SpawnLocation) const;
	void OnAttackSoundAndNiagara(const FVector& SpawnLocation) const;

	/** * @brief 무기의 타입을 지정*/
	EWeaponType WeaponType;

};

