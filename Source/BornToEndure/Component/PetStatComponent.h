#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interface/PetStatProviderInterface.h"
#include "Stat/PetStatTypes.h"
#include "Delegates/Delegate.h"
#include "PetStatComponent.generated.h"

/**
 * @brief Pet의 모든 스탯을 관리하는 컴포넌트
 * @details
 * - 기본 스탯 초기화 (PetCompanionCharacter에서 초기화 시 PetBaseDataAsset에서 Stat을 넘겨줌)
 * - Stat 증감 아이템/시너지 modifier 추가/제거
 * - 최종 스탯 계산 및 캐싱
 * - 스탯 변경 시 델리게이트로 외부 알림
 * - IPetStatProviderInterface 인터페이스를 상속받아 오버라이드함. 외부 컴포넌트와 의존성과 결합도를 낮추기 위함
 */
UCLASS(ClassGroup = (Pet), meta = (BlueprintSpawnableComponent))
class BORNTOENDURE_API UPetStatComponent : public UActorComponent, public IPetStatProviderInterface
{
    GENERATED_BODY()

public:
    UPetStatComponent();

    /**
     * @brief IPetStatProviderInterface 함수 오버라이드 구현으로 StatType에 대한 기본 스탯+아이템 스탯을 계산하여 최종 스탯 반환
	 * @param StatType EPetStatType 타입의 스탯 종류 (Attack, Defense, Speed 등)
     * @return Pet의 기본 스탯 + 아이템 스탯이 모두 적용된 최종 스탯 값
     */
    virtual float GetFinalStat(EPetStatType StatType) const override;

    /**
     * @brief 초기 기본 스탯을 초기화 하는 함수
     * @param 초기화할 기본 스탯이 저장된 FPetBaseStatSheet 구조체
	 * @details
     * - APetCompanionCharacter::BeginPlay에서 DataAsset의 BaseStats를 주입하여 1회 초기화
     */
    void InitializeBaseStats(const FPetBaseStatSheet& BaseStatSheet);

    /**
     * @brief Stat Modifier를 추가하고 해당 스탯을 재계산 후 최종 스탯 캐싱
	 * @param Modifier StatModifier 구조체로, 어떤 스탯에 어떤 방식으로 얼마만큼의 증감할 지 정의함 (AttackPower +10% 등)
     * @details
     * - UPetStatItemDataAsset::ApplyToComponent에서 아이템이 추가될 때 호출된다
     */
    void AddModifier(const FStatModifier& Modifier);

    /**
     * @brief SourceId에 해당하는 Modifier를 모두 제거하고 재계산 후 최종 스탯 캐싱
     * @param SourceId 제거하고자 하는 Stat Item의 고유 ID
     */
    void RemoveModifiersBySource(const FGuid& SourceId);

    /**
     * @brief 스탯이 변경될 때 마다 방송하는 Delegate
     * @param Delegate 타입 함수
     * @param 번경된 Stat에 대한 EPetStatType
     * @param 최종 변경된 Stat 값
     * @details
     * - PetCombatComponent에서는 해당 Delegate를 구독한 후, 공격 속도가 바뀔 때 공격 Timer를 재설정
     * - 추후 UI 등에서 사용할 수 있도록 구현
     */
    DECLARE_MULTICAST_DELEGATE_TwoParams(FOnStatChanged, EPetStatType, float /*NewValue*/);
    FOnStatChanged OnStatChanged;

protected:
    virtual void BeginPlay() override;

private:

    UPROPERTY()
    TMap<EPetStatType, float> BaseStats;

    /**
     * @brief 현재 적용이 되는 모든 Stat 증감 아이템 수치 리스트
     */
    TArray<FStatModifier> ActiveModifiers;

    /**
     * @brief 최종 Stat 계산값을 캐싱해두는 멤버 변수
     * @details
     * - mutable: const 함수인 GetFinalStat에서도 Stat 캐시를 업데이트할 수 있도록 적용
     */
    mutable TMap<EPetStatType, float> CachedFinalStats;
    mutable bool bCacheDirty = true;

    /**
     * @brief 특정 Stat에 대한 최종값을 재계산하여 캐싱하는 함수
	 * @param StatType 계산하고자 하는 Stat에 대한 EPetStatType
     * @details
     * - CachedFinalStats에서 해당 StatType을 제거한 후 재연산하고, GetFinalStat에서 다시 캐싱이 된다
     */
    void RecalculateStat(EPetStatType StatType);
    void InvalidateCache();
};