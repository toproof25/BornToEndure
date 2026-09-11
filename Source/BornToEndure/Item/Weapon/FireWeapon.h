/**
* @file FireWeapon.h
* @brief 발사체를 가지는 무기 클래스
* - BaseProjectile를 소유하여 발사체를 통해 공격을 수행하는 무기 클래스
*/
#pragma once

#include "CoreMinimal.h"
#include "Item/Weapon/BaseWeapon.h"

#include "Data/DataTableRow/WeaponItemDataRow.h"
#include "Data/CombatTypes.h"

#include "FireWeapon.generated.h"

class UPetWeaponItemDataAsset;

UCLASS()
class BORNTOENDURE_API AFireWeapon : public ABaseWeapon
{
	GENERATED_BODY()
	
public:
	virtual void OnAttack(const FPetAttackInfo& AtkInfo, const FVector& TargetLocation) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnInitalizeWeapon(const UPetWeaponItemDataAsset& ItemData) override;

	// 발사체 class -> 실질적인 객체는 오브젝트 풀링에서 관리하며, 어떤 발사체 class 타입을 사용하는지 결정
	TSubclassOf<class ABaseProjectile> ProjectileClass;
	FProjectileData ProjectileData;

	/**
	 * @brief 사용할 발사체 class를 설정하는 함수
	 * @param InProjectileClass DA에 설정된 발사체 class를 전달받아 설정
	 */
	void SetProjectileClass(TSubclassOf<class ABaseProjectile> InProjectileClass);

	/**
	 * @brief 발사체 데이터를 설정하는 함수
	 * - Combat.h에 있는 발사체 데이터 구조체를 전달받아 설정
	 * - 발사체에 대한 발사 수, 속도, 크기 등을 설정함
	 * - 발사체에 데이터를 전달하여 설정하며, 해당 함수에서는 전달 역할만 수행
	 * - 아이템 추가, 수정, 제거 시 발사체 관련 데이터 변경시에만 호출
	 * @param ProjectileData 발사체에 전달할 구조체 
	 */
	void UpdateProjectileData(const FProjectileData& InProjectileData);


private:

	// 오브젝트 풀링 초기화 함수
	void InitializeProjectilePool();


};
