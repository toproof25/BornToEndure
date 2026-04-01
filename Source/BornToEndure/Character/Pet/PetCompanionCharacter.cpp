#include "Character/Pet/PetCompanionCharacter.h"

#include "Character/Pet/PetCompanionAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h" 
#include "Component/PetCombatComponent.h"

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

	PetCombatComp = CreateDefaultSubobject<UPetCombatComponent>(TEXT("PetCombatComponent"));
}

void APetCompanionCharacter::BeginPlay()
{
	Super::BeginPlay();

	APetCompanionAIController* PetAI = Cast<APetCompanionAIController>(GetController());

	if (PetAI)
	{
		APawn* LocalPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

		if (LocalPlayer)
		{
			PetAI->SetPlayerTarget(LocalPlayer);
		}
		else
		{
			// 0.5초 뒤에 다시 시도하는 지연 탐색 추가
			FTimerHandle WaitHandle;
			GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([this, PetAI]()
				{
					APawn* DelayedPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
					if (DelayedPlayer)
					{
						PetAI->SetPlayerTarget(DelayedPlayer);
						UE_LOG(LogTemp, Log, TEXT("지연 탐색: 주인을 찾았습니다!"));
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("지연 탐색 실패: 0.5초 뒤에도 주인이 없습니다."));
					}
				}), 0.5f, false);
		}
	}
}

void APetCompanionCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void APetCompanionCharacter::OnAttack(const FVector& TargetVector)
{
	if (PetCombatComp)
	{
		PetCombatComp->OnAttack(TargetVector);
	}
}


