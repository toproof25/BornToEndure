#include "BaseProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

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
	
	// 발사체 메쉬를 콜리젼 자식으로 설정
	ProjectileMesh->SetupAttachment(SphereComp);

	// 콜리젼 초기 크기 설정 및 충돌 채널 설정
	SphereComp->InitSphereRadius(5.0f);
	//SphereComp->BodyInstance.SetCollisionProfileName("Projectile");

	// 콜리젼 Hit 이벤트 발생 시 호출할 함수 바인딩
	SphereComp->OnComponentHit.AddDynamic(this, &ABaseProjectile::OnProjectileHit);

	// 루트 컴포넌트를 콜리젼 컴포넌트로 설정
	RootComponent = SphereComp;
	
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

	// InitialLifeSpan는 AActor에서 상속받은 속성 (n초 후 자동으로 액터가 사라지도록 설정하는 변수)
	//InitialLifeSpan = ProjectileLifespan;

	SphereComp->SetSimulatePhysics(false);
	SphereComp->SetCollisionProfileName(TEXT("Projectile"));
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();
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
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
	{
		// 문서에는 물리 연산을 적용하여 밀리게 하는 기능. 일단은 주석 처리
		// OtherComp->AddImpulseAtLocation(GetVelocity() * PhysicsForce, GetActorLocation());
		UE_LOG(LogTemp, Warning, TEXT("Projectile hit: %s"), *OtherActor->GetName());
		Destroy();
	}
}

