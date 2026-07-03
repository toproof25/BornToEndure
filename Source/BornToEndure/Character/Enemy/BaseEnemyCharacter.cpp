#include "Character/Enemy/BaseEnemyCharacter.h"
#include "Subsystem/ObjectPoolSubsystem.h"
#include "Subsystem/EffectSubsystem.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/PrimaryAssetId.h"
#include "AI/Enemy/BaseEnemyAIController.h"
#include "Data/GameTypes.h"
#include "Character/Pet/PetCompanionCharacter.h"
#include "Character/Player/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"	
#include "GameFramework/DamageType.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"
#include "Components/BoxComponent.h"
#include "Data/DataTableRow/EnemyDataRow.h"
#include "Character/Player/PlayerCharacter.h"

DEFINE_LOG_CATEGORY(LogBaseEnemyCharacter);

ABaseEnemyCharacter::ABaseEnemyCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    // AI Controller 자동 소유 설정
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    // 이동 속도 초기화
    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;

	RootComponent = GetCapsuleComponent();

	AttackRangeCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackRangeCollision"));
	AttackRangeCollision->SetupAttachment(RootComponent);
}

void ABaseEnemyCharacter::InitializeEnemy(const FEnemyDataRow& EnemyData)
{
    MaxHealth = EnemyData.Health;
	CurrentHealth = MaxHealth;

    MoveSpeed = EnemyData.MovementSpeed;
    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;

    EnemyRewardPayload.ExpReward = EnemyData.RewardExp;
    EnemyRewardPayload.GoldReward = EnemyData.RewardGold;

    UE_LOG(LogBaseEnemyCharacter, Display, TEXT("ABaseEnemyCharacter::InitializeEnemy : %s | ExpReward: %.1f, GoldReward: %d"),
        *GetName(), EnemyRewardPayload.ExpReward, EnemyRewardPayload.GoldReward);
}

void ABaseEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    UWorld* World = GetWorld();
    check(World);
    UEffectSubsystem* EffectSubsystem = World->GetSubsystem<UEffectSubsystem>();
	check(EffectSubsystem);

	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ABaseEnemyCharacter::OnAttackOverlap);
	GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &ABaseEnemyCharacter::OnAttackEndOverlap);

    AttackRangeCollision->SetGenerateOverlapEvents(true);
    AttackRangeCollision->OnComponentBeginOverlap.AddDynamic(this, &ABaseEnemyCharacter::OnAttackOverlap);
    AttackRangeCollision->OnComponentEndOverlap.AddDynamic(this, &ABaseEnemyCharacter::OnAttackEndOverlap);

    // Niagara, Sound Asset Preload 및 Delegate 바인딩
    FPrimaryAssetType SoundType(TEXT("SoundDataAsset"));
    FPrimaryAssetType NiagaraType(TEXT("NiagaraDataAsset"));

    EffectSubsystem->PreloadEffectAssets(FPrimaryAssetId(SoundType, HitEnemySoundId.PrimaryAssetName));
    EffectSubsystem->PreloadEffectAssets(FPrimaryAssetId(NiagaraType, HitEnemyNiagaraId.PrimaryAssetName));

    OnEnemyHitSound.BindUObject(EffectSubsystem, &UEffectSubsystem::SpawnSoundAtLocation);
    OnEnemyHitNiagara.BindUObject(EffectSubsystem, &UEffectSubsystem::SpawnNiagaraAtLocation);;

    CurrentHealth = MaxHealth;

	Execute_ActivateActor(this);
}

void ABaseEnemyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UWorld* World = GetWorld())
    {
        if (UEffectSubsystem* EffectSubsystem = World->GetSubsystem<UEffectSubsystem>())
        {
            FPrimaryAssetType SoundType(TEXT("SoundDataAsset"));
            FPrimaryAssetType NiagaraType(TEXT("NiagaraDataAsset"));
            EffectSubsystem->UnloadEffectAssets(FPrimaryAssetId(SoundType, HitEnemySoundId.PrimaryAssetName));
            EffectSubsystem->UnloadEffectAssets(FPrimaryAssetId(NiagaraType, HitEnemyNiagaraId.PrimaryAssetName));
        }

        // 모든 이벤트 바인딩 해제
		GetCapsuleComponent()->OnComponentBeginOverlap.RemoveDynamic(this, &ABaseEnemyCharacter::OnAttackOverlap);
        GetCapsuleComponent()->OnComponentEndOverlap.RemoveDynamic(this, &ABaseEnemyCharacter::OnAttackEndOverlap);

        AttackRangeCollision->OnComponentBeginOverlap.RemoveDynamic(this, &ABaseEnemyCharacter::OnAttackOverlap);
        AttackRangeCollision->OnComponentEndOverlap.RemoveDynamic(this, &ABaseEnemyCharacter::OnAttackEndOverlap);
    }
    Super::EndPlay(EndPlayReason);
}

// 스폰(활성화)될 때 ObjectPoolSubsystem에서 호출되는 함수로, 초기화 및 활성화 로직 구현
void ABaseEnemyCharacter::ActivateActor_Implementation()
{
    // Movement 초기화
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->StopMovementImmediately();       
        MoveComp->SetMovementMode(MOVE_Walking);   
		MoveComp->MaxWalkSpeed = MoveSpeed;
    }

    // BT 초기화
    if (ABaseEnemyAIController* AICon = Cast<ABaseEnemyAIController>(GetController()))
    {
        AICon->BrainComponent->StopLogic(TEXT("PoolActivated"));

        if (BehaviorTreeAsset)
        {
			AICon->SetTargetPlayer(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
            AICon->RunBehaviorTree(BehaviorTreeAsset);
        }
    }

    // 보상 페이로드 초기화 및 스탯 초기화
    ResetRewardPayload();
    CurrentHealth = MaxHealth;

    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);

    bCanAttack = true;

    UE_LOG(LogBaseEnemyCharacter, Display, TEXT("Enemy Activated: %s"), *GetName());
}

// 제거(비활성화)될 때 ObjectPoolSubsystem에서 호출되는 함수
void ABaseEnemyCharacter::DeactivateActor_Implementation()
{
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);

    // BT 중지
    if (AAIController* AICon = Cast<AAIController>(GetController()))
    {
        AICon->BrainComponent->StopLogic(TEXT("PoolDeactivated"));
        AICon->ClearFocus(EAIFocusPriority::Gameplay);
    }

    // 위치 초기화
    SetActorLocationAndRotation(
        FVector(0.f, 0.f, -10000.f),
        FRotator::ZeroRotator,
        false, nullptr,
        ETeleportType::TeleportPhysics
    );

    // Movement 이동 중지
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->StopMovementImmediately();
    }

    UE_LOG(LogBaseEnemyCharacter, Display, TEXT("Enemy Deactivated: %s"), *GetName());
}

float ABaseEnemyCharacter::TakeDamage(
    float DamageAmount,
    struct FDamageEvent const& DamageEvent,
    AController* EventInstigator,
    AActor* DamageCauser)
{
    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // 이펙트 재생
    if (HitEnemySoundId.IsValid()) OnEnemyHitSound.ExecuteIfBound(HitEnemySoundId.PrimaryAssetName, GetActorLocation());
    if (HitEnemyNiagaraId.IsValid()) OnEnemyHitNiagara.ExecuteIfBound(HitEnemyNiagaraId.PrimaryAssetName, GetActorLocation());

    // 데미지 적용
    CurrentHealth -= ActualDamage;
    UE_LOG(LogBaseEnemyCharacter, Display, TEXT("[%s] TakeDamage: %.1f → HP: %.1f / %.1f"),
        *GetName(), ActualDamage, CurrentHealth, MaxHealth);
    
    // 데미지 정보 기록
    if (EventInstigator)
    {
        APetCompanionCharacter* Pet = Cast<APetCompanionCharacter>(EventInstigator->GetPawn());
        EnemyRewardPayload.RegisterDamage(Pet ? Pet->GetPetName() : NAME_None, ActualDamage); // 킬에 기여한 펫이 있을 경우 데미지 정보 기록
    }

    // 사망하는 경우
    if (CurrentHealth <= 0.f)
    {
		APetCompanionCharacter* Pet = Cast<APetCompanionCharacter>(EventInstigator->GetPawn());
		EnemyRewardPayload.KillerPetId = Pet ? Pet->GetPetName() : NAME_None; // 킬에 기여한 펫이 있을 경우 ID 기록
        HandleDeath();
    }

    return ActualDamage;
}

void ABaseEnemyCharacter::OnAttackOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    //if (!bCanAttack) return;

    if (!OtherActor) return;

    TargetPlayerCharacter = Cast<APlayerCharacter>(OtherActor);
	if (TargetPlayerCharacter)
    {
		UE_LOG(LogBaseEnemyCharacter, Display, TEXT("[%s] OnAttackOverlap: Player %s entered attack range."), *GetName(), *TargetPlayerCharacter->GetName());
        bIsInAttackRange = true;
        AttackPlayer();
    }

}

void ABaseEnemyCharacter::OnAttackEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor == TargetPlayerCharacter)
    {
		TargetPlayerCharacter = nullptr;
        bIsInAttackRange = false;
        GetWorld()->GetTimerManager().ClearTimer(AttackTimerHandle);
        ResetAttack();
    }
}


void ABaseEnemyCharacter::HandleDeath()
{
    // 델리게이트 브로드캐스트
    //OnEnemyDeactivated.Broadcast(this);
	OnEnemyKilled.Broadcast(EnemyRewardPayload);

    // 오브젝트 풀에 반납
    UWorld* World = GetWorld();
    if (!World) return;
    UObjectPoolSubsystem* Pool = World->GetSubsystem<UObjectPoolSubsystem>();
    if (!Pool) return;
    Pool->ReturnPoolActor(this);
}

void ABaseEnemyCharacter::AttackPlayer()
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (TargetPlayerCharacter)
    {
        // 플레이어에게 데미지 적용
        UGameplayStatics::ApplyDamage(
            TargetPlayerCharacter,
            10.f,
            GetInstigatorController(),
            this,
            UDamageType::StaticClass()
        );

        World->GetTimerManager().SetTimer(
            AttackTimerHandle,                       // 핸들 변수
            this,                                    // 함수가 속한 객체
            &ABaseEnemyCharacter::ResetAttack,      // 실행할 함수의 주소
            AttackCooldown,                                // 대기 시간 (초)
            false                                 // 반복 여부 (true = 무한 반복, false = 1회성)
        );

        bCanAttack = false;
        UE_LOG(LogBaseEnemyCharacter, Display, TEXT("[%s] AttackPlayer: Player %s hit for 10 damage."), *GetName(), *TargetPlayerCharacter->GetName());
        return;
    }
    else 
    {
        World->GetTimerManager().ClearTimer(AttackTimerHandle);
    }

    /*
	TArray<FHitResult> OverlapResults;
    World->SweepMultiByChannel(
        OverlapResults,
        AttackRangeCollision->GetComponentLocation(),
        AttackRangeCollision->GetComponentLocation(),
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeBox(AttackRangeCollision->GetScaledBoxExtent())
	);

    for (const FHitResult& Hit : OverlapResults)
    {
        if (Hit.GetActor())
        {
            if (APlayerCharacter* Player = Cast<APlayerCharacter>(Hit.GetActor()))
            {
                // 플레이어에게 데미지 적용
                UGameplayStatics::ApplyDamage(
                    Player,
                    10.f,
                    GetInstigatorController(),
                    this,
                    UDamageType::StaticClass()
                );

                World->GetTimerManager().SetTimer(
                    AttackTimerHandle,                       // 핸들 변수
                    this,                                    // 함수가 속한 객체
                    &ABaseEnemyCharacter::ResetAttack,      // 실행할 함수의 주소
                    AttackCooldown,                                // 대기 시간 (초)
                    false                                 // 반복 여부 (true = 무한 반복, false = 1회성)
                );

                bCanAttack = false;
                UE_LOG(LogBaseEnemyCharacter, Display, TEXT("[%s] AttackPlayer: Player %s hit for 10 damage."), *GetName(), *Player->GetName());
                return;
            }
        }
    }

    // Player를 찾지 못하고 공격에 실패하는 경우
	World->GetTimerManager().ClearTimer(AttackTimerHandle);
    */
}

void ABaseEnemyCharacter::ResetAttack()
{
    if (TargetPlayerCharacter)
    {
        AttackPlayer();
    }
    bCanAttack = true; 
}
