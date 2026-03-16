// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"
#include "PlayerCharacter.h"
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

	// Z축(상하)을 제외한 평면(XY) 이동 속도(GroundSpeed) 계산
	FVector Velocity = PlayerCharacter->GetVelocity();
	Velocity.Z = 0.f;
	GroundSpeed = Velocity.Size();

	// 공중에 떠 있는지 확인
	bIsFalling = MovementComponent->IsFalling();
}