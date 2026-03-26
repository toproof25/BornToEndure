// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

class APlayerCharacter;
class UCharacterMovementComponent;
class USkeletalMeshComponent;
class AWeaponBase;
enum class EWeaponType : uint8;

UCLASS()
class BORNTOENDURE_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	// 애니메이션 초기화 시 1회 호출 (BeginPlay와 유사)
	virtual void NativeInitializeAnimation() override;

	// 매 프레임 호출 (Tick과 유사) - 여기서 속도, 점프 상태를 계산
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	TObjectPtr<APlayerCharacter> PlayerCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	TObjectPtr<UCharacterMovementComponent> MovementComponent;

	// AnimBP에서 사용할 변수들
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float GroundSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsFalling;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<AWeaponBase> WeaponBaseComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	FTransform LHIKTargetTransform;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Alpha = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	EWeaponType CurrentWeaponType;

	// 위아래 고개 각도 (-90 ~ 90)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aim")
	float AimPitch;



public:

	float GetGroundSpeed() const { return GroundSpeed; }
	bool GetIsFalling() const { return bIsFalling; }
	float GetAimPitch() const { return AimPitch; }

};
