// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

// Enhanced Input 관련 헤더 포함
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"

// Camera 관련 헤더 포함
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


#include "PlayerCharacter.generated.h"

class UInputAction;
class UInputComponent;

// 각종 캐릭터 관련 액터 컴포넌트 전방 선언
class UStatComponent;
class UInteractionComponent;
class ABaseWeapon;
class USpringArmComponent;
class UCameraComponent;
class UPetManagerComponent;

UCLASS()
class BORNTOENDURE_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


protected:

	/** * @brief 카메라를 뒤에 위치시키는 Spring Arm 컴포넌트. 카메라가 캐릭터를 따라 움직이도록 설정 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<USpringArmComponent> SpringArmComp;
	
	/** * @brief 카메라를 가리키는 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UCameraComponent> CameraComp;


	/** * @brief WASD의 상하좌우 이동을 처리) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> MoveAction;

	/** * @brief Shift의 달리기를 처리) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> SprintAction;

	/** * @brief Mouse XY 회전에 카메라 Yaw, Pitch 회전을 처리 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> LookUpAction;

	/** * @brief Space bar를 이용한 Jump 처리) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> JumpAction;

	/** * @brief F를 이용한 상호작용 처리 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> InteractionAction;

	/** * @brief 마우스 좌클릭을 이용한 상호작용 처리 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> ClickLeftAction;

private:

	UFUNCTION()
	void Moving(const FInputActionValue& Value);
	UFUNCTION()
	void Sprint(const FInputActionValue& Value);

	UFUNCTION()
	void Jumping(const FInputActionValue& Value);
	UFUNCTION()
	void JumpingStop(const FInputActionValue& Value);

	UFUNCTION()
	void TurnAndLookUp(const FInputActionValue& Value);

	UFUNCTION()
	void Interact(const FInputActionValue& Value);

	UFUNCTION()
	void ClickLeft(const FInputActionValue& Value);


private:

	UPROPERTY()
	TObjectPtr<UStatComponent> StatComp;

	UPROPERTY()
	TObjectPtr<UInteractionComponent> InteractionComp;

	UPROPERTY()
	TObjectPtr<ABaseWeapon> WeaponBaseComp;


public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UPetManagerComponent> PetManagerComp;

	UFUNCTION()
	void SetWeaponBase(ABaseWeapon* NewWeaponBase) { WeaponBaseComp = NewWeaponBase; }
	
	UFUNCTION(BlueprintPure)
	void GetWeaponBase(ABaseWeapon*& OutWeaponBase) const { OutWeaponBase = WeaponBaseComp; }

	void GetPerManagerComp(UPetManagerComponent*& OutPetManagerComp) const { OutPetManagerComp = PetManagerComp; }
};


