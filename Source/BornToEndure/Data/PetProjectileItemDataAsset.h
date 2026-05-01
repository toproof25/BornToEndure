#pragma once

#include "CoreMinimal.h"
#include "Data/PetItemDataAsset.h"
#include "PetProjectileItemDataAsset.generated.h"

/**
 * @brief 발사체의 형태를 변환하는 패턴
 */
UENUM(BlueprintType)
enum class EProjectilePattern : uint8
{
    Single      UMETA(DisplayName = "단발"),
    Spread      UMETA(DisplayName = "산탄"),
    Laser       UMETA(DisplayName = "레이저"),
    Spiral      UMETA(DisplayName = "나선"),
};

/**
 * @brief 발사체 수정자 데이터
 * @note 
 * - 아이템 하나가 발사체에 미치는 영향을 기술한다.
 * - 여러 발사체 아이템이 중첩될 때를 대비하여 PetItemComponent에서 이 modifier들을 Stack을 이용하여 관리
 */
USTRUCT(BlueprintType)
struct BORNTOENDURE_API FProjectileModifierData
{
    GENERATED_BODY()

    /**
     * @brief 공격 발사체 Actor 클래스
     * @note null이면 Pet의 기본 발사체 클래스를 유지
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSoftClassPtr<AActor> OverrideProjectileClass;

    /**
     * @brief 추가 발사체 개수
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 ProjectileCountAdd = 0;           

    /**
     * @brief 추가 발사체 크기 배율
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float SizeMultiplier = 1.0f;

    /**
     * @brief 추가 스피드 배율
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float SpeedMultiplier = 1.0f;

    /**
     * @brief 공격 패턴 데이터
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    EProjectilePattern Pattern = EProjectilePattern::Single;
};

/**
 * @brief 실제로 인스턴스가 되며 공격을 변환하는 아이템 DataAsset
 */
UCLASS(BlueprintType)
class BORNTOENDURE_API UPetProjectileItemDataAsset : public UPetItemDataAsset
{
	GENERATED_BODY()

public:

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Projectile")
    FProjectileModifierData ProjectileModifier;

    virtual void ApplyToComponent(UPetItemComponent* ItemComp) const override;
    virtual void RemoveFromComponent(UPetItemComponent* ItemComp, const FGuid& InstanceId) const override;
};