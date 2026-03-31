#include "Character/Pet/PetCompanionCharacter.h"

#include "Character/Pet/PetCompanionAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
APetCompanionCharacter::APetCompanionCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// AI Controller 설정
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = APetCompanionAIController::StaticClass();

	// 캐릭터 회전 설정
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// CharacterMovement 설정
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bOrientRotationToMovement = true; 
		GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); 
		GetCharacterMovement()->MaxWalkSpeed = 400.0f;
	}

	// 3. 탐지용 스피어 컴포넌트 생성 (추후 로직 확장을 위함)
	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
	DetectionSphere->SetupAttachment(RootComponent);
	DetectionSphere->SetSphereRadius(800.0f); // 탐지 반경 설정
}

// Called when the game starts or when spawned
void APetCompanionCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// 🌟 [테스트용 안전장치] 
		// 누군가 나를 스폰하면서 SetPlayerTarget을 안 불렀다면? (즉, 에디터에 그냥 드래그해서 배치했다면)
	APetCompanionAIController* PetAI = Cast<APetCompanionAIController>(GetController());

	if (PetAI)
	{
		// 0번 로컬 플레이어 폰을 찾아서 임시로 주인으로 셋팅해 줍니다.
		APawn* LocalPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		if (LocalPlayer)
		{
			PetAI->SetPlayerTarget(LocalPlayer);
		}
	}

}

// Called every frame
void APetCompanionCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APetCompanionCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

