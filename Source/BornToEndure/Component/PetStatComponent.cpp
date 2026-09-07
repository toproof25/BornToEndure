#include "Component/PetStatComponent.h"
#include "Component/PetItemComponent.h"
#include "Character/Pet/PetCompanionCharacter.h"

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

	// 아이템 컴포넌트에서 Stat 아이템을 가져와 계산
	APetCompanionCharacter* PetCharacter = Cast<APetCompanionCharacter>(GetOwner());
	UPetItemComponent* ItemComp = PetCharacter->GetItemComponent();

    for (const FStatModifier& Mod : ItemComp->GetStatModifiers())
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
