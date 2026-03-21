// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

// Interface 상속을 위한 헤더
#include "Interactable.h"

#include "WeaponBase.generated.h"



UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_Unarmed		UMETA(DisplayName = "Unarmed"),
	EWT_Rifle		UMETA(DisplayName = "Rifle")
};

UCLASS()
class BORNTOENDURE_API AWeaponBase : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	AWeaponBase();
	virtual void Tick(float DeltaTime) override;

	/**
	 * @brief 플레이어 캐릭터와 무기 액터의 상호작용 구현
	 * @param InstigatorCharacter 상호작용을 한 플레이어 캐릭터 컴포넌트
	 */
	virtual void Interact_Implementation(APlayerCharacter* InstigatorCharacter) override;

	/**
	 * @brief 현재 무기의 타입을 저장하는 ENUM 변수
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	EWeaponType WeaponType;

protected:
	virtual void BeginPlay() override;

	/**
	 * @brief 무기 메쉬를 참조하는 포인터 
	 * @note 상호작용 시 해당 메쉬를 플레이어 소켓에 연결
	 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;
	

public:

	/**
	 * @brief 현재 클래스인 AWeaponBase 객체를 반환하는 Getter 함수
	 * @return AWeaponBase 객체의 포인터
	 */
	UFUNCTION(BlueprintPure)
	AWeaponBase* GetWeaponBase() { return this; }

	/**
	 * @brief 무기의 메쉬 컴포넌트를 반환하는 Getter 함수
	 * @return USkeletalMeshComponent를 가리키는 포인터
	 */
	UFUNCTION(BlueprintPure)
	USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
};
