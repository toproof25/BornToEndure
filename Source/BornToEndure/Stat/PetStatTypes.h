/**
 * @file PetStatTypes.h
 * @brief Pet의 모든 Stat 관련된 타입을 정의하는 헤더 파일
 * @author toproof (kmnlmn123@gmail.com)
 * @date 2026-04-20
 * @details
 * - 여러 곳에 분산되지 않고 하나의 파일에서 Stat 데이터를 정의하여 관리하기 위한 목적
 */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PetStatTypes.generated.h"


/**
 * @brief Pet의 모든 스탯 종류를 정의하는 enum class
 * @note
 * - 새로운 Stat은 해당 항목에 추가하기만 하면 된다
 */
UENUM(BlueprintType)
enum class EPetStatType : uint8
{
    // 기본 전투 스탯
    AttackPower     UMETA(DisplayName = "공격력"),
    MaxHealth       UMETA(DisplayName = "최대 체력"),
    CurrentHealth   UMETA(DisplayName = "현재 체력"),
    Defense         UMETA(DisplayName = "방어력"),
    CriticalRate    UMETA(DisplayName = "치명타 확률"),   // 0.0 ~ 1.0
    CriticalDamage  UMETA(DisplayName = "치명타 데미지"), // 배율 (기본 1.5)

    // 이동 / 공격 속도
    AttackSpeed     UMETA(DisplayName = "공격 속도"),     // 초당 공격 횟수
    MoveSpeed       UMETA(DisplayName = "이동 속도"),

    // 속성 데미지 보너스 (0.0 = 보너스 없음, 0.5 = 50% 추가)
    FireDamageBonus     UMETA(DisplayName = "화염 데미지 보너스"),
    IceDamageBonus      UMETA(DisplayName = "냉기 데미지 보너스"),
    WindDamageBonus     UMETA(DisplayName = "바람 데미지 보너스"),
    PoisonDamageBonus   UMETA(DisplayName = "독 데미지 보너스"),
    BleedDamageBonus    UMETA(DisplayName = "출혈 데미지 보너스"),
};

/**
 * @brief 각 스탯의 연산 방식을 정의하는 enum class
 * - 기본적으로 합연산 이후 곱연산을 적용한다
 */
UENUM(BlueprintType)
enum class EStatModifierType : uint8
{
    Additive        UMETA(DisplayName = "고정값 추가"),       // Base + Value
    Multiplicative  UMETA(DisplayName = "퍼센트 곱연산"),     // Base * (1 + Value)
    Override        UMETA(DisplayName = "값 덮어쓰기"),       // = Value (특수 케이스)
};


/**
 * @brief 하나의 Stat 증감 정보를 가진 구조체
 */
USTRUCT(BlueprintType)
struct BORNTOENDURE_API FStatModifier
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    EPetStatType StatType = EPetStatType::AttackPower;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    EStatModifierType ModType = EStatModifierType::Additive;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float Value = 0.f;

    // 아이템 인스턴스의 고유 ID (각 부분에서 추적하기 위해 사용)
    FGuid SourceId;
};

/**
 * @brief Pet의 기본 스탯을 구성하는 구조체
 * - PetBaseDataAsset에서 초기값을 설정하여 PetStatComponent로 전달한다
 */
USTRUCT(BlueprintType)
struct BORNTOENDURE_API FPetBaseStatSheet
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TMap<EPetStatType, float> BaseValues;
};