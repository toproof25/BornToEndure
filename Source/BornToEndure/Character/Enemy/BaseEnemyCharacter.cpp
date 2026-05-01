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

DEFINE_LOG_CATEGORY(LogBaseEnemyCharacter);

ABaseEnemyCharacter::ABaseEnemyCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    // AI Controller 자동 소유 설정
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    // 이동 속도 초기화
    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void ABaseEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    UWorld* World = GetWorld();
    check(World);
    UEffectSubsystem* EffectSubsystem = World->GetSubsystem<UEffectSubsystem>();
	check(EffectSubsystem);

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