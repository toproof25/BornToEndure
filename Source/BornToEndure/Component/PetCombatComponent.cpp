#include "Component/PetCombatComponent.h"
#include "Subsystem/EffectSubsystem.h"
#include "Subsystem/ObjectPoolSubsystem.h"
#include "Delegates/Delegate.h"
#include "Item/Projectile/BaseProjectile.h"
#include "Interface/PetStatProviderInterface.h"
#include "Interface/PetItemProviderInterface.h"
#include "UObject/PrimaryAssetId.h"
#include "TimerManager.h"
#include "AIController.h"

UPetCombatComponent::UPetCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UPetCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	check(World);
	UEffectSubsystem* EffectSubsystem = GetWorld()->GetSubsystem<UEffectSubsystem>();
	check(EffectSubsystem);

	// 사용하는 Asset Preload
	FPrimaryAssetType SoundIdType(FName(TEXT("SoundDataAsset")));
	FPrimaryAssetType NiagaraType(FName(TEXT("NiagaraDataAsset")));

	FPrimaryAssetId AttackSoundPrimaryId(SoundIdType, AttackSoundId.PrimaryAssetName);
	FPrimaryAssetId AttackNiagaraPrimaryId(NiagaraType, AttackNiagaraId.PrimaryAssetName);

	EffectSubsystem->PreloadEffectAssets(AttackSoundPrimaryId);
	EffectSubsystem->PreloadEffectAssets(AttackNiagaraPrimaryId);

	SoundDelegate.BindUObject(EffectSubsystem, &UEffectSubsystem::SpawnSoundAtLocation);
	NiagaraDelegate.BindUObject(EffectSubsystem, &UEffectSubsystem::SpawnNiagaraAtLocation);

	InitializeProjectilePool();
}

void UPetCombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UWorld* World = GetWorld();
    if (UWorld* World = GetWorld())
    {
        if (UEffectSubsystem* EffectSubsystem = GetWorld()->GetSubsystem<UEffectSubsystem>())
        {
            // 사용하는 Asset Unload
            FPrimaryAssetType SoundIdType(FName(TEXT("SoundDataAsset")));
            FPrimaryAssetType NiagaraType(FName(TEXT("NiagaraDataAsset")));

            FPrimaryAssetId AttackSoundPrimaryId(SoundIdType, AttackSoundId.PrimaryAssetName);
            FPrimaryAssetId AttackNiagaraPrimaryId(NiagaraType, AttackNiagaraId.PrimaryAssetName);

            EffectSubsystem->UnloadEffectAssets(AttackSoundPrimaryId);
            EffectSubsystem->UnloadEffectAssets(AttackNiagaraPrimaryId);
        }
    }

	// 타이머 반드시 정리 — 누락 시 컴포넌트 파괴 후 콜백이 실행되어 크래시 발생
	World->GetTimerManager().ClearTimer(AttackTimerHandle);

	Super::EndPlay(EndPlayReason);
}


void UPetCombatComponent::SetProviders(
    TScriptInterface<IPetStatProviderInterface> InStatProvider,
	TScriptInterface<IPetItemProviderInterface> InItemProvider
)
{
	StatProvider = InStatProvider;
	ItemProvider = InItemProvider;
}

void UPetCombatComponent::StartAttack()
{
	if (bIsAttacking) return;
	bIsAttacking = true;
	RefreshAttackTimer();
}

void UPetCombatComponent::StopAttack()
{
	bIsAttacking = false;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AttackTimerHandle);
	}
}

void UPetCombatComponent::OnAttack(const FVector& TargetLocation)
{
    CurrentTargetLocation = TargetLocation;

    if (!bIsAttacking)
    {
        // BTTask가 직접 호출하는 경우
        FPetAttackInfo AttackInfo = BuildAttackInfo();
        SpawnProjectiles(AttackInfo, TargetLocation);
    }
    else
    {
        // 타겟만 업데이트
        CurrentTargetLocation = TargetLocation;
    }
}

void UPetCombatComponent::OnStatChanged(EPetStatType StatType, float NewValue)
{
    if (StatType == EPetStatType::AttackSpeed && bIsAttacking)
    {
        // 공격 속도가 바뀌면 즉시 타이머 갱신
        RefreshAttackTimer();
    }
}

void UPetCombatComponent::RefreshAttackTimer()
{
    if (!StatProvider.GetObject()) return;

    UWorld* World = GetWorld();
    if (!World) return;

    const float AttackSpeed = StatProvider->GetFinalStat(EPetStatType::AttackSpeed);
    const float Interval = (AttackSpeed > 0.f) ? (1.f / AttackSpeed) : 1.f;

    // 기존 타이머 반복 주기를 교체
    World->GetTimerManager().SetTimer(
        AttackTimerHandle,
        this,
        &UPetCombatComponent::ExecuteAttack,
        Interval,
        true
    );
}

void UPetCombatComponent::SetAttackTarget(AActor* NewTarget)
{
    if (!IsValid(NewTarget)) return;

    const bool bWasAttacking = CurrentTarget.IsValid();
    CurrentTarget = NewTarget;

    // Target을 향해 회전하도록 Focus 설정
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        if (AAIController* AICon = Cast<AAIController>(OwnerPawn->GetController()))
        {
            AICon->SetFocus(NewTarget, EAIFocusPriority::Gameplay);
        }
    }

    if (!bWasAttacking)
    {
        StartAttack();
    }
}

void UPetCombatComponent::ClearAttackTarget()
{
    CurrentTarget.Reset();
    StopAttack();

    // 타겟이 사라지면 Focus 해제
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        if (AAIController* AICon = Cast<AAIController>(OwnerPawn->GetController()))
        {
            AICon->ClearFocus(EAIFocusPriority::Gameplay);
        }
    }
}

void UPetCombatComponent::ExecuteAttack()
{
    if (!CurrentTarget.IsValid())
    {
        StopAttack();
        return;
    }

    FPetAttackInfo AttackInfo = BuildAttackInfo();
    SpawnProjectiles(AttackInfo, CurrentTarget->GetActorLocation());
}

FPetAttackInfo UPetCombatComponent::BuildAttackInfo() const
{
    FPetAttackInfo Info;

    // StatProvider에서 공격력, 크리티컬 관련 스탯 아이템을 종합하여 적용
    if (StatProvider.GetObject())
    {
        const float AttackPower = StatProvider->GetFinalStat(EPetStatType::AttackPower);
        const float CritRate = StatProvider->GetFinalStat(EPetStatType::CriticalRate);
        const float CritDamage = StatProvider->GetFinalStat(EPetStatType::CriticalDamage);

        // 크리티컬 판정 연산
        Info.bIsCritical = FMath::FRand() < CritRate;
        Info.CriticalMultiplier = Info.bIsCritical ? CritDamage : 1.f;
        Info.FinalDamage = AttackPower * Info.CriticalMultiplier;
    }

    // ItemProvider에서 발사체 아이템 종합 - 발사체에 대한 패턴, 크기, 속도 등을 정의함
    if (ItemProvider.GetObject())
    {
        const FProjectileModifierData Modifier = ItemProvider->GetAggregatedProjectileModifier();
        Info.ElementTag = ItemProvider->GetDominantElementTag();

        // 아이템이 Override를 지정했으면 사용, 없으면 기본값
        if (!Modifier.OverrideProjectileClass.IsNull())
        {
            // Soft class를 동기 로드
            Info.ProjectileClass = Modifier.OverrideProjectileClass.Get();
        }
        else
        {
            Info.ProjectileClass = DefaultProjectileClass;
        }

        Info.ProjectileCount = FMath::Max(1, Modifier.ProjectileCountAdd);
        Info.ProjectileSize = Modifier.SizeMultiplier;
        Info.ProjectileSpeed = 1200.f * Modifier.SpeedMultiplier;
        Info.Pattern = Modifier.Pattern;
    }
    else
    {
        // 아이템 없으면 기본값
        Info.ProjectileClass = DefaultProjectileClass;
        Info.ProjectileCount = 1;
    }

    return Info;
}

void UPetCombatComponent::SpawnProjectiles(
    const FPetAttackInfo& AttackInfo,
    const FVector& TargetLocation)
{
    if (!AttackInfo.ProjectileClass) return;

    // 오브젝트 풀 시스템 가져오기
    UWorld* World = GetWorld();
    if (World == nullptr) return;
    UObjectPoolSubsystem* ObjectPoolSubsystem = World->GetSubsystem<UObjectPoolSubsystem>();
	if (ObjectPoolSubsystem == nullptr) return;

    // 사용할 발사체 오브젝트 풀에서 가져오기
    UClass* ProjectileClassKey = DefaultProjectileClass.Get();

    const FVector Origin = GetOwner()->GetActorLocation();
    const TArray<FVector> Directions = CalculateProjectileDirections(
        AttackInfo.Pattern, AttackInfo.ProjectileCount, Origin, TargetLocation);

    for (const FVector& Dir : Directions)
    {
        AActor* GetPoolActor = ObjectPoolSubsystem->RequestPoolActor(AttackInfo.ProjectileClass);
        if (GetPoolActor == nullptr) return;
        ABaseProjectile* Projectile = Cast<ABaseProjectile>(GetPoolActor);
        if (Projectile == nullptr) return;

        // 발사체에 대한 소유자와 주체자 초기화
        Projectile->Owner = GetOwner();
        Projectile->SetInstigator(Cast<APawn>(GetOwner()));
        Projectile->SetActorLocationAndRotation(Origin, Dir.Rotation(), false, nullptr, ETeleportType::TeleportPhysics);
		Projectile->SetActorScale3D(FVector(AttackInfo.ProjectileSize));

        // 사운드 및 Niagara 재생
        OnAttackSound(Origin);
        OnAttackNiagara(Origin);

        // 발사체 발사
		Projectile->SetHomingTarget(CurrentTarget.Get());
        Projectile->FireProjectile(AttackInfo, Dir);

        //AActor* Projectile = World->SpawnActor<AActor>(AttackInfo.ProjectileClass, Origin, Dir.Rotation(), SpawnParams);

        // 발사체 인터페이스를 통해 공격 정보 전달
        // (IPetProjectileInterface는 추후 구현)
        // if (IPetProjectileInterface* ProjInterface = Cast<IPetProjectileInterface>(Projectile))
        // {
        //     ProjInterface->InitializeWithAttackInfo(AttackInfo);
        // }
    }
}

TArray<FVector> UPetCombatComponent::CalculateProjectileDirections(
    EProjectilePattern Pattern,
    int32 Count,
    const FVector& Origin,
    const FVector& TargetLocation) const
{
    TArray<FVector> Directions;
    const FVector BaseDir = (TargetLocation - Origin).GetSafeNormal();

    switch (Pattern)
    {
        case EProjectilePattern::Single:
            Directions.Add(BaseDir);
            break;

        case EProjectilePattern::Spread:
        {
            // Count발을 일정 각도로 펼친다
            const float SpreadAngle = 15.f;
            const float AngleStep = (Count > 1) ? (SpreadAngle * 2.f / (Count - 1)) : 0.f;
            for (int32 i = 0; i < Count; ++i)
            {
                const float Angle = -SpreadAngle + AngleStep * i;
                const FVector Dir = BaseDir.RotateAngleAxis(Angle, FVector::UpVector);
                Directions.Add(Dir);
            }
            break;
        }

        case EProjectilePattern::Spiral:
        {
            const float AngleStep = 360.f / Count;
            for (int32 i = 0; i < Count; ++i)
            {
                const FVector Dir = BaseDir.RotateAngleAxis(AngleStep * i, FVector::UpVector);
                Directions.Add(Dir);
            }
            break;
        }

        default:
            Directions.Add(BaseDir);
            break;
    }

    return Directions;
}

void UPetCombatComponent::OnAttackSound(const FVector& SpawnLocation) const
{
	if (AttackSoundId.IsValid())
	{
		SoundDelegate.ExecuteIfBound(AttackSoundId.PrimaryAssetName, SpawnLocation);
	}
}

void UPetCombatComponent::OnAttackNiagara(const FVector& SpawnLocation) const
{
	if (AttackNiagaraId.IsValid())
	{
		NiagaraDelegate.ExecuteIfBound(AttackNiagaraId.PrimaryAssetName, SpawnLocation);
	}
}

void UPetCombatComponent::InitializeProjectilePool()
{
	if (ProjectilePoolSize <= 0) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UObjectPoolSubsystem* ObjectPoolSubsystem = World->GetSubsystem<UObjectPoolSubsystem>();
	if (ObjectPoolSubsystem)
	{
		UClass* Projectile = DefaultProjectileClass.Get();
		ObjectPoolSubsystem->InitializePoolForClass(DefaultProjectileClass, ProjectilePoolSize);
	}
}