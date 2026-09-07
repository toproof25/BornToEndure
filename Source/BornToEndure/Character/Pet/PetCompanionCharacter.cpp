#include "Character/Pet/PetCompanionCharacter.h"

#include "Character/Pet/PetCompanionAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h" 
#include "Component/PetCombatComponent.h"
#include "Component/PetStatComponent.h"
#include "Component/PetItemComponent.h"
#include "Data/PetBaseDataAsset.h"
#include "Data/PetItemDataAsset.h"
#include "Data/PetProjectileItemDataAsset.h"
#include "Subsystem/ItemPoolSubsystem.h"
#include "Data/DataTableRow/WeaponItemDataRow.h"

APetCompanionCharacter::APetCompanionCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// AI Controller 설정
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = APetCompanionAIController::StaticClass();

	// Movement 설정
	bUseControllerRotationYaw = false;
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);
		GetCharacterMovement()->MaxWalkSpeed = 400.f;
	}

	// 각종 컴포넌트 생성
	PetStatComp = CreateDefaultSubobject<UPetStatComponent>(TEXT("PetStatComponent"));
	PetItemComp = CreateDefaultSubobject<UPetItemComponent>(TEXT("PetItemComponent"));
	PetCombatComp = CreateDefaultSubobject<UPetCombatComponent>(TEXT("PetCombatComponent"));

	// 콜리전 설정 (탐지용으로만 설정)
	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
	DetectionSphere->SetupAttachment(RootComponent);
	DetectionSphere->SetSphereRadius(800.f);
	DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

}

void APetCompanionCharacter::BeginPlay()
{
	Super::BeginPlay();

	// DataAsset이 설정되어 있을 때만 초기화
	if (PetBaseData)
	{
		InitializeFromDataAsset();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[PetCompanionCharacter] %s: PetBaseData is not set! Using default values."), *GetName());
	}

	// 컴포넌트 간 델리게이트 연결
	BindComponentDelegates();
}


void APetCompanionCharacter::InitializeFromDataAsset()
{
	// Item Subsystem에서 StarWeaponID를 가져온 후 인스턴스화 하여 시작 아이템으로 적용
	UWorld* World = GetWorld();
	if (!World) return;
	UItemPoolSubsystem* ItemPoolSubsystem = World ? World->GetGameInstance()->GetSubsystem<UItemPoolSubsystem>() : nullptr;
	if (!ItemPoolSubsystem) return;

	// 시작 무기의 경우 초기에 가져와서 설정
	const FWeaponItemDataRow* StartWeaponRow = ItemPoolSubsystem->GetWeaponItemDataRowByID(PetBaseData->StartWeaponID);
	TSoftObjectPtr<UPetItemDataAsset> StartWeaponDataAsset = StartWeaponRow->WeaponItemDataAsset;
	UPetProjectileItemDataAsset* StartWeaponInstance = Cast<UPetProjectileItemDataAsset>(StartWeaponDataAsset.LoadSynchronous());

	FItemDataHandle StartWeaponHandle;
	StartWeaponHandle.ItemType = EItemType::Weapon;
	StartWeaponHandle.ItemRowName = PetBaseData->StartWeaponID;

	// StatComponent 초기화
	if (PetStatComp)
	{
		PetStatComp->InitializeBaseStats(PetBaseData->BaseStats);
	}

	// CombatComponent에 기본 공격 클래스 설정 (BeginPlay에서는 로드하여 적용)
	if (PetCombatComp && StartWeaponInstance)
	{
		FProjectileModifierData ProjectileModifier = StartWeaponInstance->ProjectileModifier;
		PetCombatComp->DefaultProjectileClass = ProjectileModifier.OverrideProjectileClass.LoadSynchronous();
	}

	if (PetItemComp)
	{
		PetItemComp->AddItem(StartWeaponHandle);
	}

	// 이동 속도도 DataAsset 기반으로 설정
	if (GetCharacterMovement() && PetStatComp)
	{
		GetCharacterMovement()->MaxWalkSpeed = PetStatComp->GetFinalStat(EPetStatType::MoveSpeed);
	}
}

void APetCompanionCharacter::BindComponentDelegates()
{
	if (!PetStatComp || !PetCombatComp) return;

	// StatComponent의 스탯 변경 이벤트를 CombatComponent에 전달
	PetStatComp->OnStatChanged.AddUObject(PetCombatComp, &UPetCombatComponent::OnStatChanged);

	// 이동 속도 변경 시 CharacterMovement에 반영 (람다로 직접 바인딩)
	PetStatComp->OnStatChanged.AddLambda(
		[this](EPetStatType StatType, float NewValue)
		{
			if (StatType == EPetStatType::MoveSpeed && GetCharacterMovement())
			{
				GetCharacterMovement()->MaxWalkSpeed = NewValue;
			}
		}
	);

	// TScriptInterface은 Interface에서 사용하는 TObjectPtr같은 기능
	// CombatComp에 캐싱된 StatComp, ItemComp를 전달하여 CombatComp가 Stat과 Item 정보를 가져갈 수 있도록 함
	PetCombatComp->SetProviders(
		TScriptInterface<IPetStatProviderInterface>(PetStatComp),
		TScriptInterface<IPetItemProviderInterface>(PetItemComp)
	);
}


void APetCompanionCharacter::OnAttack(const FVector& TargetVector)
{
	if (PetCombatComp)
	{
		PetCombatComp->OnAttack(TargetVector);
	}
}

void APetCompanionCharacter::SetFollowOwner(APawn* PlayerPawn)
{
	if (PlayerPawn)
	{
		APetCompanionAIController* PetAI = Cast<APetCompanionAIController>(GetController());
		if (PetAI)
		{
			PetAI->SetPlayerTarget(PlayerPawn);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("플레이어 설정 실패: AI Controller가 없습니다."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("플레이어 설정 실패: PlayerPawn이 nullptr입니다."));
	}
}

UBehaviorTree* APetCompanionCharacter::GetBehaviorTree() const
{
	if (PetBaseData && !PetBaseData->BehaviorTree.IsNull())
	{
		return PetBaseData->BehaviorTree.LoadSynchronous();
	}
	return nullptr;
}

FName APetCompanionCharacter::GetPetName() const
{
	FText Name = PetBaseData->PetName;
	FName NameAsFName(*Name.ToString());
	return NameAsFName;
}

TSoftObjectPtr<UTexture2D> APetCompanionCharacter::GetIcon() const
{
	return PetBaseData ? PetBaseData->Icon : nullptr;
}
