// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseEnemy.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"

DEFINE_LOG_CATEGORY(LogBaseEnemy);

ABaseEnemy::ABaseEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	EnemyMesh = CreateDefaultSubobject <UStaticMeshComponent>(TEXT("EnemyMesh"));
	EnemyColision = CreateDefaultSubobject <UCapsuleComponent>(TEXT("EnemyColision"));


	EnemyMesh->SetupAttachment(EnemyColision);

	EnemyColision->InitCapsuleSize(40.0f, 92.0f);


	RootComponent = EnemyColision;

}

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();
	

	UE_LOG(LogBaseEnemy, Warning, TEXT("Spawn Test Enemy!!"));
}

void ABaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

float ABaseEnemy::TakeDamage(
	float DamageAmount,
	struct FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser
)
{
	// 대충 물리 방어력 적용했다고 가정
	DamageAmount -= 5;

	FString CauserName = DamageCauser ? DamageCauser->GetName() : TEXT("Unknown");
	UE_LOG(LogBaseEnemy, Warning, TEXT("Enemy took damage: %f, Attacker: %s"), DamageAmount, *CauserName);

	return DamageAmount;
}
