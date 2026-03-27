#include "BaseProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "ObjectPoolSubsystem.h"
#include "Engine/GameInstance.h"

DEFINE_LOG_CATEGORY(LogBaseProjectile);

ABaseProjectile::ABaseProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	// 각 필요 컴포넌트를 생성
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	check(ProjectileMesh != nullptr);

	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMesh->SetCollisionProfileName(TEXT("NoCollision"));

	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	check(ProjectileMovementComp != nullptr);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	check(SphereComp != nullptr);

	// 발사체 메쉬를 콜리젼 자식으로 설정
	ProjectileMesh->SetupAttachment(SphereComp);

	// 콜리젼 초기 크기 설정 및 충돌 채널 설정
	SphereComp->InitSphereRadius(5.0f);

	// 루트 컴포넌트를 콜리젼 컴포넌트로 설정
	RootComponent = SphereComp;

	// 이동, 충돌 연산을 ShpereComp에 적용하도록 설정
	ProjectileMovementComp->UpdatedComponent = SphereComp;

	// 발사체 초기 설정
	ProjectileMovementComp->InitialSpeed = 3000.f;
	ProjectileMovementComp->MaxSpeed = 3000.f;
	ProjectileMovementComp->bRotationFollowsVelocity = true;
	ProjectileMovementComp->bSweepCollision = true;
	ProjectileMovementComp->ProjectileGravityScale = 0.f;

	ProjectileMovementComp->bShouldBounce = false;
	ProjectileMovementComp->Bounciness = 0.0f;
	ProjectileMovementComp->Friction = 0.0f;


	SphereComp->SetSimulatePhysics(false);
	SphereComp->SetCollisionProfileName(TEXT("Projectile"));
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();

	UWorld* world = GetWorld();
	if (world == nullptr) return;
	UObjectPoolSubsystem* ObjectPoolSubsystem = world->GetSubsystem<UObjectPoolSubsystem>();
	if (ObjectPoolSubsystem)
	{
		TimerDelegate.BindUObject(ObjectPoolSubsystem, &UObjectPoolSubsystem::ReturnPoolActor, Cast<AActor>(this));
	}

	if (SphereComp)
	{
		SphereComp->OnComponentHit.AddDynamic(this, &ABaseProjectile::OnProjectileHit);
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


	if (ProjectileMovementComp)
	{
		ProjectileMovementComp->StopMovementImmediately();
		ProjectileMovementComp->SetUpdatedComponent(SphereComp);
	}

	UE_LOG(LogBaseProjectile, Display, TEXT("ActivateProjectile: %s"), *GetName());
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

}

void ABaseProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseProjectile::OnProjectileHit(
	UPrimitiveComponent* HitComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit
)
{

	UE_LOG(LogBaseProjectile, Display, TEXT("OnProjectileHit called. OtherActor: %s"), OtherActor ? *OtherActor->GetName() : TEXT("nullptr"));

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
	}
}

