#include "BaseProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
<<<<<<< HEAD
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "ObjectPoolSubsystem.h"

DEFINE_LOG_CATEGORY(LogBaseProjectile);
=======
>>>>>>> main

ABaseProjectile::ABaseProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	// 각 필요 컴포넌트를 생성
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	check(ProjectileMesh != nullptr);

	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	check(ProjectileMovementComp != nullptr);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	check(SphereComp != nullptr);
<<<<<<< HEAD

=======
	
>>>>>>> main
	// 발사체 메쉬를 콜리젼 자식으로 설정
	ProjectileMesh->SetupAttachment(SphereComp);

	// 콜리젼 초기 크기 설정 및 충돌 채널 설정
	SphereComp->InitSphereRadius(5.0f);
	//SphereComp->BodyInstance.SetCollisionProfileName("Projectile");

	// 콜리젼 Hit 이벤트 발생 시 호출할 함수 바인딩
	SphereComp->OnComponentHit.AddDynamic(this, &ABaseProjectile::OnProjectileHit);

	// 루트 컴포넌트를 콜리젼 컴포넌트로 설정
	RootComponent = SphereComp;
<<<<<<< HEAD

=======
	
>>>>>>> main
	// 이동, 충돌 연산을 ShpereComp에 적용하도록 설정
	ProjectileMovementComp->UpdatedComponent = SphereComp;

	// 발사체 초기 설정
	ProjectileMovementComp->InitialSpeed = 3000.f;
	ProjectileMovementComp->MaxSpeed = 3000.f;
	ProjectileMovementComp->bRotationFollowsVelocity = true;
	ProjectileMovementComp->ProjectileGravityScale = 0.f;

	ProjectileMovementComp->bShouldBounce = false;
	ProjectileMovementComp->Bounciness = 0.0f;
	ProjectileMovementComp->Friction = 0.0f;

<<<<<<< HEAD
	SphereComp->SetSimulatePhysics(false);
	SphereComp->SetCollisionProfileName(TEXT("Projectile"));
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	DamageType = UDamageType::StaticClass();
=======
	// InitialLifeSpan는 AActor에서 상속받은 속성 (n초 후 자동으로 액터가 사라지도록 설정하는 변수)
	//InitialLifeSpan = ProjectileLifespan;

	SphereComp->SetSimulatePhysics(false);
	SphereComp->SetCollisionProfileName(TEXT("Projectile"));
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
>>>>>>> main
}

void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();
<<<<<<< HEAD

	UWorld* world = GetWorld();
	if (world == nullptr) return;
	UObjectPoolSubsystem* ObjectPoolSubsystem = world->GetSubsystem<UObjectPoolSubsystem>();
	if (ObjectPoolSubsystem)
	{
		TimerDelegate.BindUObject(ObjectPoolSubsystem, &UObjectPoolSubsystem::ReturnPoolActor, Cast<AActor>(this));
	}

}

void ABaseProjectile::ActivateActor_Implementation()
{
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	// ProjectileLifespan 시간 뒤 자동 반환 타이머
	if (TimerDelegate.IsBound())
	{
		GetWorld()->GetTimerManager().SetTimer(
			LifeSpanTimerHandle,
			TimerDelegate,
			ProjectileLifespan,
			false
		);
	}

	if (SphereComp)
	{
		SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	ProjectileMovementComp->StopMovementImmediately();
	ProjectileMovementComp->SetUpdatedComponent(SphereComp); // 이동 대상 재확인
}

void ABaseProjectile::DeactivateActor_Implementation()
{
	// 모습과 콜리젼 끄기
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	// 위치 초기화
	SetActorLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator, false, nullptr, ETeleportType::TeleportPhysics);

	// 발사체 멈춤, Tick 비활성화
	if (ProjectileMovementComp)
	{
		ProjectileMovementComp->StopMovementImmediately();
		ProjectileMovementComp->Deactivate();
	}

	// 충돌 끄기
	if (SphereComp)
	{
		SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	UE_LOG(LogBaseProjectile, Display, TEXT("ReturnProjectile: %s"), *GetName());
=======
>>>>>>> main
}

void ABaseProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseProjectile::OnProjectileHit(
<<<<<<< HEAD
	UPrimitiveComponent* HitComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit
)
{
	// 부모 클래스에서 사용한 발사체를 풀로 반환
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
	{
		// 타이머 초기화 후 반환
		GetWorld()->GetTimerManager().ClearTimer(LifeSpanTimerHandle);
		UWorld* world = GetWorld();
		if (world == nullptr) return;
		UObjectPoolSubsystem* ObjectPoolSubsystem = world->GetSubsystem<UObjectPoolSubsystem>();
		if (ObjectPoolSubsystem)
		{
			ObjectPoolSubsystem->ReturnPoolActor(this);
		}

=======
	UPrimitiveComponent* HitComp, 
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, 
	FVector NormalImpulse, 
	const FHitResult& Hit
)
{
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
	{
		// 문서에는 물리 연산을 적용하여 밀리게 하는 기능. 일단은 주석 처리
		// OtherComp->AddImpulseAtLocation(GetVelocity() * PhysicsForce, GetActorLocation());
		UE_LOG(LogTemp, Warning, TEXT("Projectile hit: %s"), *OtherActor->GetName());
		Destroy();
>>>>>>> main
	}
}

