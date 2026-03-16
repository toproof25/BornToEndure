// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"
#include "PlayerCharacter.h"

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;

	// 물리 충돌 허용
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetCollisionProfileName(TEXT("BlockAll"));
}

void AWeaponBase::Interact_Implementation(APlayerCharacter* InstigatorCharacter)
{
	if (InstigatorCharacter == nullptr) return;

	// 무기 메쉬의 물리와 콜리젼 제거
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 플레이어 메쉬와 소켓에 무기 부착
	USkeletalMeshComponent* PlayerMesh = InstigatorCharacter->GetMesh();
	if (PlayerMesh)
	{
		AttachToComponent(PlayerMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("WeaponSocket"));
	}

	UE_LOG(LogTemp, Warning, TEXT("Weapon Interacted: %s"), *InstigatorCharacter->GetName());
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();

}

void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

