// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Delegates/Delegate.h"
#include "Interface/Interactable.h" // Interface 상속을 위한 헤더
#include "BaseWeapon.generated.h"

class UStaticMeshComponent;

// Delegate 선언
DECLARE_DELEGATE_TwoParams(FSpawnSoundAtLocation, FName, FVector);
DECLARE_DELEGATE_TwoParams(FSpawnNiagaraAtLocation, FName, FVector);

// 로그 카테고리 선언
DECLARE_LOG_CATEGORY_EXTERN(LogBaseWeapon, Log, All);


UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_Unarmed		UMETA(DisplayName = "Unarmed"),
	EWT_Rifle		UMETA(DisplayName = "Rifle")
};

UCLASS(Abstract)
class BORNTOENDURE_API ABaseWeapon : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	ABaseWeapon();
	virtual void Tick(float DeltaTime) override;

	/**
	 * @brief 플레이어 캐릭터와 무기 액터의 상호작용 구현
	 * @param InstigatorCharacter 상호작용을 한 플레이어 캐릭터 컴포넌트
	 */
	virtual void Interact_Implementation(APlayerCharacter* InstigatorCharacter) override;

protected:
	virtual void BeginPlay() override;

	/**
	 * @brief 무기 메쉬를 참조하는 포인터
	 * @note 상호작용 시 해당 메쉬를 플레이어 소켓에 연결
	 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> WeaponStaticMesh;

public:
	/**
	 * @brief UProjectilePoolSubsystem에서 사용할 풀 크기를 설정하는 변수
	 */
	UPROPERTY(EditAnywhere, Category = "Weapon")
	int32 PoolSize;

	/** * @brief 현재 무기의 타입을 저장하는 ENUM 변수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	EWeaponType WeaponType;

	/**
	 * @brief 현재 클래스인 ABaseWeapon 객체를 반환하는 Getter 함수
	 * @return ABaseWeapon 객체의 포인터
	 */
	UFUNCTION(BlueprintPure)
	ABaseWeapon* GetWeaponBase() { return this; }

	/**
	 * @brief 무기의 메쉬 컴포넌트를 반환하는 Getter 함수
	 * @return USkeletalMeshComponent를 가리키는 포인터
	 */
	UFUNCTION(BlueprintPure)
	UStaticMeshComponent* GetWeaponStaticMesh() const { return WeaponStaticMesh; }

	/**
	 * @brief Delegate 시그니처 선언으로, 공격 시 사운드와 나이아가라 스폰 호출
	 */
	FSpawnSoundAtLocation SoundDelegate;
	FSpawnNiagaraAtLocation NiagaraDelegate;

	/**
	 * @brief 사운드와 나이아가라 애셋을 참조하는 변수
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|Sound", meta = (AllowedTypes = "SoundDataAsset"))
	FPrimaryAssetId AttackSoundId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|Niagara", meta = (AllowedTypes = "NiagaraDataAsset"))
	FPrimaryAssetId AttackNiagaraId;

	UFUNCTION()
	void OnAttackSound(const FVector& SpawnLocation) const;

	UFUNCTION()
	void OnAttackNiagara(const FVector& SpawnLocation) const;

public:
	/**
	 * @brief 공격 함수로 플레이어 캐릭터가 공격 입력을 받았을 때 호출되는 함수
	 * @note 자식 클래스에서 이 함수를 오버라이드하여 실제 공격 로직을 구현할 수 있음
	 */
	UFUNCTION()
	virtual void Attack() PURE_VIRTUAL(ABaseWeapon::Attack, );

	/**
	 * @brief UProjectilePoolSubsystem으로 발사체 풀을 초기화하는 함수
	 */
	virtual void InitializeProjectilePool() {};
};

