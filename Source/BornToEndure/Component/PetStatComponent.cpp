#include "Component/PetStatComponent.h"

UPetStatComponent::UPetStatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UPetStatComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UPetStatComponent::InitializeBaseStats(const FPetBaseStatSheet& BaseStatSheet)
{
    BaseStats = BaseStatSheet.BaseValues;
    InvalidateCache();

    UE_LOG(LogTemp, Log, TEXT("[PetStatComponent] Base stats initialized. Count: %d"), BaseStats.Num());
}

float UPetStatComponent::GetFinalStat(EPetStatType StatType) const
{
    // 캐시가 유효하면 캐시에서 바로 반환
    if (!bCacheDirty)
    {
        if (const float* Cached = CachedFinalStats.Find(StatType))
        {
            return *Cached;
        }
    }

    // 특정 스탯 재계산
    const float Base = BaseStats.FindRef(StatType);

    float AdditiveSum = 0.f;
    float MultiplicativeProduct = 1.f;

    for (const FStatModifier& Mod : ActiveModifiers)
    {
        if (Mod.StatType != StatType) continue;

        switch (Mod.ModType)
        {
            case EStatModifierType::Additive:
                AdditiveSum += Mod.Value;
                break;
            case EStatModifierType::Multiplicative:
                // 곱연산 중첩
                MultiplicativeProduct *= (1.f + Mod.Value);
                break;
            case EStatModifierType::Override:
                // Override는 그 즉시 반환 (다른 계산 무시)
                return Mod.Value;
        }
    }

    // 최종값 = (기본 + 합) * 곱
    const float FinalValue = (Base + AdditiveSum) * MultiplicativeProduct;
    CachedFinalStats.Add(StatType, FinalValue);

    return FinalValue;
}

void UPetStatComponent::AddModifier(const FStatModifier& Modifier)
{
    ActiveModifiers.Add(Modifier);
    RecalculateStat(Modifier.StatType);
}


void UPetStatComponent::RemoveModifiersBySource(const FGuid& SourceId)
{
    // 1. 가진 모든 아이템에서 일치하는 타입들을 모두 수집 (중복은 없도록 Set으로 사용)
    TSet<EPetStatType> AffectedStats;
    for (const FStatModifier& Mod : ActiveModifiers)
    {
        if (Mod.SourceId == SourceId)
        {
            AffectedStats.Add(Mod.StatType);
        }
    }

    // 2. SourceId가 일치하는 modifier 모두 제거
    ActiveModifiers.RemoveAllSwap([&SourceId](const FStatModifier& Mod)
        {
            return Mod.SourceId == SourceId;
        }
    );

    // 3. 영향받은 스탯만 재계산
    for (EPetStatType StatType : AffectedStats)
    {
        RecalculateStat(StatType);
    }
}

void UPetStatComponent::RecalculateStat(EPetStatType StatType)
{
    // 해당 스탯의 캐시를 무효화하고 새 값을 계산
    CachedFinalStats.Remove(StatType);
    bCacheDirty = false; // 부분 재계산이므로 전체 dirty는 아님

    const float NewValue = GetFinalStat(StatType);

    // 구독자들에게 변경 알림
    OnStatChanged.Broadcast(StatType, NewValue);
}

void UPetStatComponent::InvalidateCache()
{
    CachedFinalStats.Empty();
    bCacheDirty = true;
}