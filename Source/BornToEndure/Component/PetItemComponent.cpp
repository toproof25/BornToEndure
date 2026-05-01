#include "Component/PetItemComponent.h"
#include "Component/PetStatComponent.h"
#include "Data/PetItemDataAsset.h"
#include "Data/PetSynergyDataAsset.h"
#include "Character/Pet/PetCompanionCharacter.h"
#include "Engine/AssetManager.h"
#include "Subsystem/ObjectPoolSubsystem.h"


UPetItemComponent::UPetItemComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UPetItemComponent::BeginPlay()
{
    Super::BeginPlay();
    LoadSynergyDataAsync();
}

void UPetItemComponent::AddItem(UPetItemDataAsset* ItemData)
{
    if (!ItemData) return;

    // 1. 아이템 인스턴스 생성
    FPetItemInstance NewInstance;
    NewInstance.InstanceId = FGuid::NewGuid();
    NewInstance.LoadedData = ItemData;
    LastAddedInstanceId = NewInstance.InstanceId;

    OwnedItems.Add(NewInstance);

    // 2. 아이템 DataAsset에게 적용을 위임 (Visitor 패턴)
        // StatItemData라면 → StatComponent에 modifier를 추가
        // ProjectileItemData라면 → ItemComponent의 ProjectileModifier에 추가
    ItemData->ApplyToComponent(this);

    // 3. 시너지 재검사
    CheckAndUpdateSynergies();

    // 4. 외부에 방송
    OnItemAdded.Broadcast(ItemData);

    UE_LOG(LogTemp, Log, TEXT("[PetItemComponent] Item added: %s (InstanceId: %s)"),
        *ItemData->ItemName.ToString(), *LastAddedInstanceId.ToString());
}

void UPetItemComponent::RemoveItem(const FGuid& InstanceId)
{
    // 1.소유한 아이템에서 FGuid가 일치하는 아이템의 Index를 찾는다
    const int32 Index = OwnedItems.IndexOfByPredicate([&InstanceId](const FPetItemInstance& Inst)
        {
            return Inst.InstanceId == InstanceId;
        });

    if (Index == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PetItemComponent] RemoveItem: InstanceId not found."));
        return;
    }

    // 2.찾은 Index를 기반으로 DataAsset을 가져온다
    UPetItemDataAsset* ItemData = OwnedItems[Index].LoadedData;
    if (ItemData)
    {
		// 3.FGuid를 기반으로 아이템을 각 Component에서 제거한다
        ItemData->RemoveFromComponent(this, InstanceId);
    }

    // 4.최종적으로 소유한 아이템에서 제거할 아이템에 해당하는 Index를 제거한다
    OwnedItems.RemoveAtSwap(Index);

    // 5.시너지를 재계산한 후 외부에 방송한다
    CheckAndUpdateSynergies();
    OnItemRemoved.Broadcast(InstanceId);
}

void UPetItemComponent::AddProjectileModifier(const FProjectileModifierData& Modifier,
    const FGuid& InstanceId)
{
    ProjectileModifiers.Add(InstanceId, Modifier);
    
    // 새로운 발사체 오브젝트 풀링
    UWorld* World = GetWorld();
    if(!World) return;
    UObjectPoolSubsystem* ObjectPoolSubsystem = World->GetSubsystem<UObjectPoolSubsystem>();
    if (ObjectPoolSubsystem)
    {
		UClass* Projectile = Modifier.OverrideProjectileClass.Get();
        ObjectPoolSubsystem->InitializePoolForClass(Projectile, 10);
    }
}

void UPetItemComponent::RemoveProjectileModifier(const FGuid& InstanceId)
{

    // 새로운 발사체 오브젝트 풀링에서 제거
    UWorld* World = GetWorld();
    if (!World) return;
    UObjectPoolSubsystem* ObjectPoolSubsystem = World->GetSubsystem<UObjectPoolSubsystem>();
    if (ObjectPoolSubsystem)
    {
        UClass* Projectile = ProjectileModifiers[InstanceId].OverrideProjectileClass.Get();
        if (Projectile)
        {
            ObjectPoolSubsystem->RemovePoolActor(Projectile);
        }
    }

    ProjectileModifiers.Remove(InstanceId);
}

FProjectileModifierData UPetItemComponent::GetAggregatedProjectileModifier() const
{
    // 1.기본 발사체를 바탕으로 결과 발사체를 만든다
    FProjectileModifierData Result;
    Result.ProjectileCountAdd = 1; // 기본 1발에서 시작

    // 2.ProjectileModifiers의 모든 요소를 순회
    for (const auto& [Id, Modifier] : ProjectileModifiers)
    {
        // 발사체 클래스 교체: 마지막에 추가된 것으로 덮어쓴다
        if (!Modifier.OverrideProjectileClass.IsNull())
        {
            Result.OverrideProjectileClass = Modifier.OverrideProjectileClass;
        }

        // 추가 발사, 크기, 스피드 등 연산
        Result.ProjectileCountAdd += Modifier.ProjectileCountAdd;
        Result.SizeMultiplier *= Modifier.SizeMultiplier;
        Result.SpeedMultiplier *= Modifier.SpeedMultiplier;

        // 패턴: 가장 마지막(우선순위 높은) 것이 적용
        if (Modifier.Pattern != EProjectilePattern::Single)
        {
            Result.Pattern = Modifier.Pattern;
        }
    }

    return Result;
}

FGameplayTag UPetItemComponent::GetDominantElementTag() const
{
    // 현재 소유한 모든 아이템의 시너지 태그를 집계
    TMap<FGameplayTag, int32> TagCounts;
    for (const FPetItemInstance& Instance : OwnedItems)
    {
        if (!Instance.LoadedData) continue;
        for (const FGameplayTag& Tag : Instance.LoadedData->SynergyTags)
        {
            TagCounts.FindOrAdd(Tag)++;
        }
    }

    // 집계한 시너지 태그 중 가장 빈도가 높은 태그를 반환
    FGameplayTag DominantTag;
    int32 MaxCount = 0;
    for (const auto& [Tag, Count] : TagCounts)
    {
        if (Count > MaxCount)
        {
            MaxCount = Count;
            DominantTag = Tag;
        }
    }

    return DominantTag;
}

void UPetItemComponent::CheckAndUpdateSynergies()
{
    if (AllSynergyData.IsEmpty()) return;

    // 현재 아이템의 모든 태그 집계
    TMap<FGameplayTag, int32> CurrentTagCounts;
    for (const FPetItemInstance& Instance : OwnedItems)
    {
        if (!Instance.LoadedData) continue;
        for (const FGameplayTag& Tag : Instance.LoadedData->SynergyTags)
        {
            CurrentTagCounts.FindOrAdd(Tag)++;
        }
    }

    // StatComponent를 포인터 변수로 임시로 가져옴
    APetCompanionCharacter* PetChar = Cast<APetCompanionCharacter>(GetOwner());
    UPetStatComponent* StatComp = PetChar ? PetChar->GetStatComponent() : nullptr;

    // 초기에 게임에 존재하는 모든 시너지 데이터를 미리 로드한 `AllSynergyData`을 순회
    // 현재 집계한 태그에서 시너지 조건이 만족하는 시너지를 적용하는 방식
    for (UPetSynergyDataAsset* SynergyData : AllSynergyData)
    {
        if (!SynergyData) continue;

        const bool bShouldBeActive = IsSynergyConditionMet(SynergyData, CurrentTagCounts);
        const bool bCurrentlyActive = ActiveSynergies.Contains(SynergyData);

		// 활성화 가능한 시너지는 존재하면서 현재 활성화 상태가 아닌 경우 -> 활성화
        if (bShouldBeActive && !bCurrentlyActive)
        {
            // 시너지를 활성화하기 위해 Add
            ActiveSynergies.Add(SynergyData);
            if (StatComp)
            {
                // StatComponent에 시너지 Stat 변화 적용
                for (FStatModifier Bonus : SynergyData->StatBonuses)
                {
                    Bonus.SourceId = FGuid::NewGuid();
                    StatComp->AddModifier(Bonus);
                }
            }
            OnSynergyChanged.Broadcast(SynergyData, true);
            UE_LOG(LogTemp, Log, TEXT("[PetItemComponent] Synergy activated: %s"),
                *SynergyData->SynergyName.ToString());
        }

		// 활성화 가능한 시너지가 없으면서 현재 활성화된 시너지가 존재하는 경우 -> 비활성화
        else if (!bShouldBeActive && bCurrentlyActive)
        {
            // 시너지 비활성화
            ActiveSynergies.Remove(SynergyData);

            // 시너지 modifier 제거는 별도 SourceId 추적 로직 필요
            // (심화 구현 단계에서 완성)
            OnSynergyChanged.Broadcast(SynergyData, false);
        }
    }
}

bool UPetItemComponent::IsSynergyConditionMet(
    const UPetSynergyDataAsset* SynergyData,
    const TMap<FGameplayTag, int32>& CurrentTagCounts) const
{
    // 시너지 데이터 자체가 없거나 조건이 만족하는게 없다면 false
    // 활성화 가능한 시너지가 1개라도 존재한다면 true

    for (const auto& [RequiredTag, RequiredCount] : SynergyData->RequiredTagCounts)
    {
        const int32 CurrentCount = CurrentTagCounts.FindRef(RequiredTag);
        if (CurrentCount < RequiredCount)
        {
            return false; // 하나라도 조건 미달이면 시너지 불성립
        }
    }
    return !SynergyData->RequiredTagCounts.IsEmpty();
}

void UPetItemComponent::LoadSynergyDataAsync()
{
    // AssetManager를 통해 "PetSynergy" 카테고리의 모든 에셋을 비동기 로드
    UAssetManager& AM = UAssetManager::Get();

    TArray<FPrimaryAssetId> SynergyIds;
    AM.GetPrimaryAssetIdList(FPrimaryAssetType("PetSynergy"), SynergyIds);

    if (SynergyIds.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("[PetItemComponent] No synergy data assets found."));
        return;
    }

    AM.LoadPrimaryAssets(SynergyIds, {},
        FStreamableDelegate::CreateUObject(this, &UPetItemComponent::OnSynergyDataLoaded, SynergyIds));
}

void UPetItemComponent::OnSynergyDataLoaded(TArray<FPrimaryAssetId> LoadedIds)
{
    UAssetManager& AM = UAssetManager::Get();
    AllSynergyData.Empty();

    for (const FPrimaryAssetId& Id : LoadedIds)
    {
        if (UPetSynergyDataAsset* Data = Cast<UPetSynergyDataAsset>(AM.GetPrimaryAssetObject(Id)))
        {
            AllSynergyData.Add(Data);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[PetItemComponent] Synergy data loaded. Count: %d"), AllSynergyData.Num());
}