#include "Component/PetItemComponent.h"
#include "Component/PetStatComponent.h"
#include "Data/PetItemDataAsset.h"
#include "Data/PetSynergyDataAsset.h"
#include "Character/Pet/PetCompanionCharacter.h"
#include "Engine/AssetManager.h"

#include "Data/DataTableRow/ItemDataRow.h"
#include "Data/DataTableRow/StatItemDataRow.h"
#include "Data/DataTableRow/WeaponItemDataRow.h"

#include "Component/PetCombatComponent.h"

#include "Subsystem/ObjectPoolSubsystem.h"
#include "Subsystem/ItemPoolSubsystem.h"



UPetItemComponent::UPetItemComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UPetItemComponent::BeginPlay()
{
    Super::BeginPlay();
    LoadSynergyDataAsync();
}

void UPetItemComponent::AddItem(FItemDataHandle ItemData)
{
	// 1. 아이템 인스턴스 생성
	FPetItemInstance NewInstance;
	NewInstance.InstanceId = FGuid::NewGuid();
	NewInstance.ItemDataHandle = ItemData;
	LastAddedInstanceId = NewInstance.InstanceId;

	UWorld* World = GetWorld();
	if (World == nullptr) return;
	UItemPoolSubsystem* ItemPoolSubsystem = World->GetGameInstance()->GetSubsystem<UItemPoolSubsystem>();
	if (ItemPoolSubsystem == nullptr) return;

	switch (ItemData.ItemType)
	{
		case EItemType::Stat:
		{
			const FStatItemDataRow* StatItemDataRow = ItemPoolSubsystem->GetStatItemDataRowByID(ItemData.ItemRowName);

			// 해당 아이템에 대한 FGuid를 모두 추가한 후 StatComponent에 Modifier로 추가한다 (Stat 추적 가능하도록)
			for (FStatModifier Mod : StatItemDataRow->StatModifiers)
			{
				Mod.SourceId = LastAddedInstanceId;
				UE_LOG(LogTemp, Log, TEXT("[PetItemComponent] AddItem: Adding StatModifier for SourceId: %s, StatType: %d, Value: %f"),
					*Mod.SourceId.ToString(), static_cast<int32>(Mod.StatType), Mod.Value);
				AddStatModifier(Mod);
			}
			break;
		}

		case EItemType::Weapon:
		{
			const FWeaponItemDataRow* WeaponItemDataRow = ItemPoolSubsystem->GetWeaponItemDataRowByID(ItemData.ItemRowName);
			UPetCombatComponent* CombatComp = Cast<APetCompanionCharacter>(GetOwner())->GetCombatComponent();
			if (CombatComp)
			{
				CombatComp->InitializeDefaultWeapon(*WeaponItemDataRow);
			}
			// 무기의 경우 비동기로 로드 후 ApplyToComponent 호출하도록 함
			//UPetItemDataAsset* WeaponItemData = WeaponItemDataRow->WeaponItemDataAsset.LoadSynchronous();
			//NewInstance.LoadedData = WeaponItemData;
			//WeaponItemData->ApplyToComponent(this);
			break;
		}
		default:
			break;
	}

	OwnedItems.Add(NewInstance);

	// 2. 시너지 재검사
	CheckAndUpdateSynergies();

	// 3. 외부에 방송
	//OnItemAdded.Broadcast(ItemData);

	//UE_LOG(LogTemp, Log, TEXT("[PetItemComponent] Item added: %s (InstanceId: %s)"), *ItemData->ItemName.ToString(), *LastAddedInstanceId.ToString());
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
	else
	{
		// 스탯 아이템의 경우 ItemData가 nullptr로 존재함
		RemoveStatModifiersBySource(InstanceId);
	}

    // 4.최종적으로 소유한 아이템에서 제거할 아이템에 해당하는 Index를 제거한다
    OwnedItems.RemoveAtSwap(Index);

    // 5.시너지를 재계산한 후 외부에 방송한다
    CheckAndUpdateSynergies();
    OnItemRemoved.Broadcast(InstanceId);
}

/*
void UPetItemComponent::AddWeaponModifier(const FWeaponModifierData& Modifier, const FGuid& InstanceId)
{
    WeaponModifiers.Add(InstanceId, Modifier);
    
    // 새로운 발사체 오브젝트 풀링
    UWorld* World = GetWorld();
    if(!World) return;
    UObjectPoolSubsystem* ObjectPoolSubsystem = World->GetSubsystem<UObjectPoolSubsystem>();
    if (ObjectPoolSubsystem)
    {
		UClass* Weapon = Modifier.OverrideWeaponClass.Get();
        ObjectPoolSubsystem->InitializePoolForClass(Weapon, 10);
    }
}

void UPetItemComponent::RemoveWeaponModifier(const FGuid& InstanceId)
{
    // 새로운 발사체 오브젝트 풀링에서 제거
    UWorld* World = GetWorld();
    if (!World) return;
    UObjectPoolSubsystem* ObjectPoolSubsystem = World->GetSubsystem<UObjectPoolSubsystem>();
    if (ObjectPoolSubsystem)
    {
        UClass* Weapon = WeaponModifiers[InstanceId].OverrideWeaponClass.Get();
        if (Weapon)
        {
            ObjectPoolSubsystem->RemovePoolActor(Weapon);
        }
    }

    WeaponModifiers.Remove(InstanceId);
}
*/

void UPetItemComponent::AddStatModifier(const FStatModifier& Modifier)
{
	ActiveStatModifiers.Add(Modifier);

	APetCompanionCharacter* PetChar = Cast<APetCompanionCharacter>(GetOwner());
	UPetStatComponent* StatComp = PetChar->GetStatComponent();
	StatComp->RecalculateStat(Modifier.StatType);
}

void UPetItemComponent::RemoveStatModifiersBySource(const FGuid& SourceId)
{
	// 1. 가진 모든 아이템에서 일치하는 타입들을 모두 수집 (중복은 없도록 Set으로 사용)
	TSet<EPetStatType> AffectedStats;
	for (const FStatModifier& Mod : ActiveStatModifiers)
	{
		if (Mod.SourceId == SourceId)
		{
			AffectedStats.Add(Mod.StatType);
		}
	}

	// 2. SourceId가 일치하는 modifier 모두 제거
	ActiveStatModifiers.RemoveAllSwap([&SourceId](const FStatModifier& Mod)
		{
			return Mod.SourceId == SourceId;
		}
	);

	// 3. 영향받은 스탯만 재계산
	APetCompanionCharacter* PetChar = Cast<APetCompanionCharacter>(GetOwner());
	UPetStatComponent* StatComp = PetChar->GetStatComponent();
	for (EPetStatType StatType : AffectedStats)
	{
		StatComp->RecalculateStat(StatType);
	}
}
/*
FWeaponModifierData UPetItemComponent::GetAggregatedWeaponModifier() const
{
    // 1.기본 발사체를 바탕으로 결과 발사체를 만든다
    FWeaponModifierData Result;
    Result.WeaponCountAdd = 1; // 기본 1발에서 시작

    // 2.WeaponModifiers의 모든 요소를 순회
    for (const auto& [Id, Modifier] : WeaponModifiers)
    {
        // 발사체 클래스 교체: 마지막에 추가된 것으로 덮어쓴다
        if (!Modifier.OverrideWeaponClass.IsNull())
        {
            Result.OverrideWeaponClass = Modifier.OverrideWeaponClass;
        }

        // 추가 발사, 크기, 스피드 등 연산
        Result.WeaponCountAdd += Modifier.WeaponCountAdd;
        Result.SizeMultiplier *= Modifier.SizeMultiplier;
        Result.SpeedMultiplier *= Modifier.SpeedMultiplier;
		Result.ElementType = Modifier.ElementType;

        // 패턴: 가장 마지막(우선순위 높은) 것이 적용
        if (Modifier.Pattern != EWeaponPattern::Single)
        {
            Result.Pattern = Modifier.Pattern;
        }
    }

    return Result;
}
*/
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

    // 현재 아이템의 모든 태그 집계 *********************** 아마 Stat 아이템은 적용이 안될 수 있음 리펙토링 하면서 구조가 바뀌어서
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
                    AddStatModifier(Bonus);
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

const TArray<FItemDataHandle> UPetItemComponent::GetOwnedItemRowHandles() const
{
	TArray<FItemDataHandle> ItemRowHandles;
	for (const FPetItemInstance& ItemInstance : OwnedItems)
	{
		ItemRowHandles.Add(ItemInstance.ItemDataHandle);
	}
	return ItemRowHandles;
}
