#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interface/PetItemProviderInterface.h"
#include "Data/PetProjectileItemDataAsset.h"
#include "GameplayTagContainer.h"
#include "PetItemComponent.generated.h"

class UPetItemDataAsset;
class UPetSynergyDataAsset;

/**
 * @brief 아이템 인스턴스를 표현하는 구조체
 * @details
 * - DataAsset 포인터와 고유 ID(를 함께 관리하여 같은 아이템이라도 각각의 아이템으로 취급
 * - 같은 아이템 DataAsset을 두 번 획득하면 InstanceId가 다르기에 독립적으로 존재한다
 */
USTRUCT()
struct FPetItemInstance
{
    GENERATED_BODY()

    // 이 인스턴스의 고유 ID
    FGuid InstanceId;

    // 실제 아이템 데이터
    UPROPERTY()
    TObjectPtr<UPetItemDataAsset> LoadedData = nullptr;
};


/**
 * @brief Pet이 보유한 모든 아이템을 관리하는 컴포넌트
 * @details
 * - 아이템 추가/제거
 * - 아이템 추가/제거 시 StatComponent에 modifier 반영 (DataAsset의 Apply/Remove 호출)
 * - 아이템 추가마다 시너지 조건 검사
 * - 발사체 modifier 집계
 */
UCLASS(ClassGroup = (Pet), meta = (BlueprintSpawnableComponent))
class BORNTOENDURE_API UPetItemComponent : public UActorComponent, public IPetItemProviderInterface
{
    GENERATED_BODY()

public:
    UPetItemComponent();

    // IPetItemProviderInterface 오버라이드 구현
    virtual FProjectileModifierData GetAggregatedProjectileModifier() const override;
    virtual FGameplayTag GetDominantElementTag() const override;

    /**
     * @brief 아이템 추가
     * @param ItemData PetManagerComponent에서 아이템 DataAsset을 보내준다
     * @details
	 * - 파라미터로 온 아이템의 FPetItemInstance를 생성한 후 OwnedItems에 추가한다
	 * - 각 ItemData의 ApplyToComponent를 호출하여 적절하게 반영됨
     */
    void AddItem(UPetItemDataAsset* ItemData);

    /**
     * @brief 아이템 제거
     * @param InstanceId 제거할 Id의 고유 FGuid값 
     * @details
     * - 보유 아이템 OwnedItems에서 같인 ID를 찾아 제거한다 
	 * - 제거할 아이템 DataAsset의 RemoveFromComponent를 호출하여 적절하게 반영됨
     */
    void RemoveItem(const FGuid& InstanceId);

    FGuid GetLastAddedInstanceId() const { return LastAddedInstanceId; }

    /**
     * @brief 보유 아이템의 발사체 modifier를 추가한다.
     * - UPetProjectileItemData::ApplyToComponent에서 호출한다.
     */

    /**
	 * @brief ProjectileModifier를 TMap에 추가
     * @param Modifier 새로 추가할 발사체에 대한 modifire 구조체
     * @param InstanceId 추가할 발사체의 고유 ID
     */
    void AddProjectileModifier(const FProjectileModifierData& Modifier, const FGuid& InstanceId);

    /**
     * @brief Projectile modifier를 제거
     * @param InstanceId 제거할 modifire의 고유한 FGuid
     */
    void RemoveProjectileModifier(const FGuid& InstanceId);

	// 아이템 추가, 제거, 시너지 변경 Delegate (UI 업데이트 등에 활용하여 아직 구독된 곳 없음)
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemAdded, const UPetItemDataAsset*);
    FOnItemAdded OnItemAdded;

    DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemRemoved, const FGuid&);
    FOnItemRemoved OnItemRemoved;

    DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSynergyChanged, const UPetSynergyDataAsset*, bool);
    FOnSynergyChanged OnSynergyChanged;

protected:
    virtual void BeginPlay() override;

private:

    TArray<FPetItemInstance> OwnedItems;
    TMap<FGuid, FProjectileModifierData> ProjectileModifiers;

    /** @brief활성화된 시너지 목록 */
    UPROPERTY()
    TArray<TObjectPtr<UPetSynergyDataAsset>> ActiveSynergies;

    /** 
    * @brief 모든 시너지 DataAsset로 게임에 존재하는 모든 시너지를 미리 캐싱한다
    * - 추후 아이템 획득 시 비교하기 위해 사용함
    */
    UPROPERTY()
    TArray<TObjectPtr<UPetSynergyDataAsset>> AllSynergyData;

    FGuid LastAddedInstanceId;

    /**
     * @brief 현재 보유 아이템의 모든 SynergyTag를 집계하고 AllSynergyData와 비교하여 현재 적용된 시너지 상태를 업데이트
     */
    void CheckAndUpdateSynergies();

    /**
     * @brief 적용할 시너지의 조건이 현재 보유 아이템에서 존재하는지 체크하여 여부를 반환
     * - 모든 시너지를 순회하며 각 시너지별로 조건을 체크
     * @param SynergyData 적용할 시너지에 대한 DataAsset
     * @param CurrentTagCounts 현재 보유 아이템에서 집계된 모든 속성 테그 모음
     * @return `SynergyData` 시너지가 적용될 여부를 반환
     */
    bool IsSynergyConditionMet(
        const UPetSynergyDataAsset* SynergyData,
        const TMap<FGameplayTag, int32>& CurrentTagCounts ) const;

    /**
     * @brief 시너지 DataAsset을 AssetManager를 통해 비동기 로드
     * - BeginPlay에서 한 번만 호출
     */
    void LoadSynergyDataAsync();
    void OnSynergyDataLoaded(TArray<FPrimaryAssetId> LoadedIds);

public:

    // 디버그 및 UI용 Getter
    const TArray<FPetItemInstance>& GetOwnedItems() const { return OwnedItems; }
    const TArray<TObjectPtr<UPetSynergyDataAsset>>& GetActiveSynergies() const { return ActiveSynergies; }
};