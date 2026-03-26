// Fill out your copyright notice in the Description page of Project Settings.


#include "RifleWeapon.h"
#include "Components/ArrowComponent.h"
#include "BaseProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
<<<<<<< HEAD
#include "ObjectPoolSubsystem.h"
#include "Components/SphereComponent.h"
=======
>>>>>>> main


ARifleWeapon::ARifleWeapon()
{
<<<<<<< HEAD
	PrimaryActorTick.bCanEverTick = false;
=======
	PrimaryActorTick.bCanEverTick = true;
>>>>>>> main

	ProjectilePoint = CreateDefaultSubobject<UArrowComponent>(TEXT("ProjectilePoint"));
	ProjectilePoint->SetupAttachment(RootComponent); 
}

void ARifleWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void ARifleWeapon::Attack()
{
    if (ProjectileClass == nullptr || ProjectilePoint == nullptr) return;

    UWorld* World = GetWorld();
    if (World == nullptr) return;
<<<<<<< HEAD
    UObjectPoolSubsystem* ObjectPoolSubsystem = World->GetSubsystem<UObjectPoolSubsystem>();
    
=======

>>>>>>> main
    // 발사체 스폰 위치와 회전 설정
    FVector SpawnLocation = ProjectilePoint->GetComponentLocation();
    FRotator SpawnRotation = ProjectilePoint->GetComponentRotation();

<<<<<<< HEAD
    // 사용할 발사체 가져오기
    UClass* ProjectileClassKey = ProjectileClass.Get();
    AActor* GetPoolActor = ObjectPoolSubsystem->RequestPoolActor(ProjectileClassKey);
    if (GetPoolActor == nullptr) return;
    ABaseProjectile* Projectile = Cast<ABaseProjectile>(GetPoolActor);
    if (Projectile == nullptr) return;

    // 소유자와 주체자 초기화
	Projectile->Owner = GetOwner();
    Projectile->SetInstigator(GetInstigator());

	// 이동 시 발사체가 충돌하지 않도록 위치와 회전을 설정 (위치 초기화)
    Projectile->SetActorLocationAndRotation(SpawnLocation, SpawnRotation, false, nullptr, ETeleportType::TeleportPhysics);

    // 발사체 컴포넌트를 찾아 발사
	UProjectileMovementComponent* ProjectileMovementComp;
    Projectile->GetProjectileMovementComponent(ProjectileMovementComp);
    if (ProjectileMovementComp)
    {
        FVector LaunchDirection = SpawnRotation.Vector();
        ProjectileMovementComp->Velocity = LaunchDirection * ProjectileMovementComp->InitialSpeed;
        ProjectileMovementComp->Activate(true);
=======
    // 스폰 설정
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = GetOwner();
    SpawnParams.Instigator = GetInstigator();
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

    // 스폰 위치와 회전으로 발사체를 생성
    ABaseProjectile* Projectile = World->SpawnActor<ABaseProjectile>(
        ProjectileClass,
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );

    if (Projectile == nullptr) return;

    // 발사체 컴포넌트를 찾아 발사
	UProjectileMovementComponent* ProjectileMovementComp;
    Projectile->GetProjectileMovementComponent(ProjectileMovementComp);

    if (ProjectileMovementComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("InitialSpeed: %f"), ProjectileMovementComp->InitialSpeed);
        UE_LOG(LogTemp, Warning, TEXT("Velocity before: %s"), *ProjectileMovementComp->Velocity.ToString());

        FVector LaunchDirection = SpawnRotation.Vector();
        ProjectileMovementComp->Velocity = LaunchDirection * ProjectileMovementComp->InitialSpeed;

        UE_LOG(LogTemp, Warning, TEXT("Velocity after: %s"), *ProjectileMovementComp->Velocity.ToString());

        Projectile->SetLifeSpan(Projectile->ProjectileLifespan);
        UE_LOG(LogTemp, Warning, TEXT("Projectile lifespan: %f"), Projectile->InitialLifeSpan);

        // SpawnRotation에서 직접 방향 벡터 출력
        UE_LOG(LogTemp, Warning, TEXT("SpawnRotation: %s"), *SpawnRotation.ToString());
        UE_LOG(LogTemp, Warning, TEXT("LaunchDirection (unit): %s"), *SpawnRotation.Vector().GetSafeNormal().ToString());
>>>>>>> main
    }
}

void ARifleWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ARifleWeapon::InitializeProjectilePool()
{
    if (PoolSize <= 0) return;

    UWorld* World = GetWorld();
    if (!World) return;

    UObjectPoolSubsystem* ObjectPoolSubsystem = World->GetSubsystem<UObjectPoolSubsystem>();
    if (ObjectPoolSubsystem)
    {
		UClass* Projectile = ProjectileClass.Get();
        ObjectPoolSubsystem->InitializePoolForClass(ProjectileClass, PoolSize);
    }
}
