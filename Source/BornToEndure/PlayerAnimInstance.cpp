// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"
#include "PlayerCharacter.h"
#include "WeaponBase.h"
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
	AWeaponBase* CurrentWeapon;
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


	// 캐릭터가 바라보는 시선 회전값 (카메라 컨트롤러 기준)
	FRotator AimRotation = PlayerCharacter->GetBaseAimRotation();

	// 캐릭터 액터의 실제 회전값
	FRotator ActorRotation = PlayerCharacter->GetActorRotation();

	// 두 회전값의 차이를 구하여 로컬 회전 각도 도출 (-180 ~ 180 범위로 정규화)
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, ActorRotation);

	// FInterpTo를 사용해 값이 튀지 않고 부드럽게 보간되도록 적용
	AimPitch = FMath::FInterpTo(AimPitch, DeltaRot.Pitch, DeltaSeconds, 15.0f);

}