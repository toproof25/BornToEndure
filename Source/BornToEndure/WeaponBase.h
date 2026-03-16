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
	EWT_Unarmed		UMETA(DisplayName = "맨손"),
	EWT_Rifle		UMETA(DisplayName = "소총")
};

UCLASS()
class BORNTOENDURE_API AWeaponBase : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	AWeaponBase();

	virtual void Interact_Implementation(APlayerCharacter* InstigatorCharacter) override;

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
	virtual void Tick(float DeltaTime) override;

};
