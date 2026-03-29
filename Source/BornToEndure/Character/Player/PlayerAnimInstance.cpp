// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/PlayerAnimInstance.h"
#include "Character/Player/PlayerCharacter.h"
#include "Item/Weapon/BaseWeapon.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

void UPlayerAnimInstance::NativeInitializeAnimation()
{

	Super::NativeInitializeAnimation();

	// 애니메이션을 소유한 폰(Pawn)을 내 캐릭터로 캐스팅하여 캐싱
	PlayerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());

	if (PlayerCharacter)
	{
		MovementComponent = PlayerCharacter->GetCharacterMovement();
	}

}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (PlayerCharacter == nullptr || MovementComponent == nullptr) return;

	// 상하를 제외한 XY 이동 속도 계산
	FVector Velocity = PlayerCharacter->GetVelocity();
	Velocity.Z = 0.f;
	GroundSpeed = Velocity.Size();

	// 공중에 떠 있는지 확인
	bIsFalling = MovementComponent->IsFalling();


	// PlayerCharacter에서 AWaeponBase를 가지고 있는 지 확인
	ABaseWeapon* CurrentWeapon;
	PlayerCharacter->GetWeaponBase(CurrentWeapon);

	if (CurrentWeapon && CurrentWeapon->GetWeaponMesh())
	{
		// 무기를 들고 있다면, "LHIK" 소켓 좌표를 계속 저장
		WeaponBaseComp = CurrentWeapon;
		LHIKTargetTransform = CurrentWeapon->GetWeaponMesh()->GetSocketTransform(FName("LHIK"), ERelativeTransformSpace::RTS_World);
		Alpha = 1.0f; // IK 적용
		CurrentWeaponType = WeaponBaseComp->WeaponType;
	}
	else
	{
		WeaponBaseComp = nullptr;
		Alpha = 0.0f; // IK 적용 안함
		CurrentWeaponType = EWeaponType::EWT_Unarmed;
	}

}