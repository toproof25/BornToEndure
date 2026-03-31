// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Player/PlayerCharacter.h"
#include "GameFramework/Character.h"

// Enhanced Input 관련 헤더 포함
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"

// 각종 캐릭터 관련 액터 컴포넌트
#include "Component/StatComponent.h"
#include "Component/InteractionComponent.h"
#include "Item/Weapon/BaseWeapon.h"

// Camera 관련 헤더 포함
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// USpringArmComponent, UCameraComponent를 생성
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));

	// 카메라 위치를 설정
	GetMesh()->SetRelativeLocationAndRotation(
		FVector(0.0f, 0.0f, -90.0f), 
		FQuat(FRotator(0.0f, -90.0f, 0.0f))
	);

	// 현재 매쉬에 자식으로 붙여 고정 
	SpringArmComp->SetupAttachment(RootComponent);
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);

	// Spring Arm이 캐릭터 회전을 따라가도록 설정
	SpringArmComp->bUsePawnControlRotation = true;


	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bIgnoreBaseRotation = false;

	// 컨트롤러가 원하는 회전을 사용하도록 활성화
	GetCharacterMovement()->bUseControllerDesiredRotation = true;

}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	StatComp = FindComponentByClass<UStatComponent>();
	StatComp->RecalculateSpeed(GetCharacterMovement(), false);

	InteractionComp = FindComponentByClass<UInteractionComponent>();
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// PlayerInputComponent를 Enhanced Input을 사용하기 위해 UEnhancedInputComponent로 캐스팅
	if (UEnhancedInputComponent* Input = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Moving);

		Input->BindAction(SprintAction, ETriggerEvent::Started, this, &APlayerCharacter::Sprint);
		Input->BindAction(SprintAction, ETriggerEvent::Canceled, this, &APlayerCharacter::Sprint);
		Input->BindAction(SprintAction, ETriggerEvent::Completed, this, &APlayerCharacter::Sprint);

		Input->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayerCharacter::Jumping);
		Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &APlayerCharacter::JumpingStop);

		Input->BindAction(LookUpAction, ETriggerEvent::Triggered, this, &APlayerCharacter::TurnAndLookUp);
		
		Input->BindAction(InteractionAction, ETriggerEvent::Started, this, &APlayerCharacter::Interact);

		Input->BindAction(ClickLeftAction, ETriggerEvent::Started, this, &APlayerCharacter::ClickLeft);


		 

		UE_LOG(LogTemp, Log, TEXT("Binding MoveAction to Moving function"));
	}
	
}

void APlayerCharacter::Moving(const FInputActionValue& Value)
{
	if (Controller == nullptr) return;

	// 입력된 Axixs 2D값을 벡터를 2D 벡터로 추출
	FVector2D MovementVector = Value.Get<FVector2D>();
	//UE_LOG(LogTemp, Log, TEXT("Move Action Triggered with Value: %s"), *MovementVector.ToString());

	// 컨트롤러 회전에서 Yaw(수평면)만 추출
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// 전방 방향과 오른쪽 방향 벡터 구하기
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);  // X 축이 앞으로
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);    // Y 축이 오른쪽

	// AddMovementInput으로 이동 입력을 게임플레이 시스템에 전달
	AddMovementInput(ForwardDirection, MovementVector.Y);  // Y는 앞뒤
	AddMovementInput(RightDirection, MovementVector.X);    // X는 좌우
}

void APlayerCharacter::Sprint(const FInputActionValue& Value)
{
	if (Controller == nullptr) return;
	bool bIsSprinting = Value.Get<bool>();
	StatComp->RecalculateSpeed(GetCharacterMovement(), bIsSprinting);
	//UE_LOG(LogTemp, Log, TEXT("Sprint Action Triggered with Value: %s"), bIsSprinting ? TEXT("Run") : TEXT("Not Run"));
}


void APlayerCharacter::Jumping(const FInputActionValue& Value)
{
	if (Controller == nullptr) return;

	bool bJumpCheck = Value.Get<bool>();

	if (bJumpCheck)
	{
		//UE_LOG(LogTemp, Log, TEXT("Jump Action Triggered with Value: %s"), bJumpCheck ? TEXT("True") : TEXT("False"));
		Jump();
	}
}

void APlayerCharacter::JumpingStop(const FInputActionValue& Value)
{
	if (Controller == nullptr) return;

	bool bJumpCheck = Value.Get<bool>();

	if (!bJumpCheck)
	{
		//UE_LOG(LogTemp, Log, TEXT("Jump Stop Action Triggered with Value: %s"), bJumpCheck ? TEXT("True") : TEXT("False"));
		StopJumping();
	}
}

void APlayerCharacter::TurnAndLookUp(const FInputActionValue& Value)
{
	if (Controller == nullptr) return;

	FVector2D LookAxisVector = Value.Get<FVector2D>();
	//UE_LOG(LogTemp, Warning, TEXT("Look Vector: X=%.3f, Y=%.3f"), LookAxisVector.X, LookAxisVector.Y);

	// 마우스 X축 좌우 회전 (Yaw)
	AddControllerYawInput(LookAxisVector.X);

	// 마우스 Y축 상하 회전 (Pitch) 
	AddControllerPitchInput(LookAxisVector.Y);
}

void APlayerCharacter::Interact(const FInputActionValue& Value)
{
	if (Controller == nullptr) return;

	UE_LOG(LogTemp, Log, TEXT("Interaction Action Triggered"));
	if (InteractionComp)
	{
		InteractionComp->TestInteraction();
	}
}

void APlayerCharacter::ClickLeft(const FInputActionValue& Value)
{
	if (Controller == nullptr) return;

	bool bClickLeftCheck = Value.Get<bool>();

	//UE_LOG(LogTemp, Log, TEXT("ClickLeft Action with Value: %s"), bClickLeftCheck ? TEXT("True") : TEXT("False"));

	if (bClickLeftCheck && WeaponBaseComp)
	{
		WeaponBaseComp->Attack();
	}
}

