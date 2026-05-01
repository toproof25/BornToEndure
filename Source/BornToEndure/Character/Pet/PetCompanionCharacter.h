#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTree.h"
#include "PetCompanionCharacter.generated.h"

class USphereComponent;
class UPetCombatComponent;
class UPetStatComponent;
class UPetItemComponent;
class UPetBaseDataAsset;


/**
 * @brief Pet의 본체 캐릭터 클래스
 * - Pet이 사용하는 컴포넌트들을 생성하고 캐싱 변수로 보유
 * - DataAsset(UPetBaseData)을 읽어 컴포넌트들을 초기화
 * - 외부에서 컴포넌트에 접근하는 Getter 제공
 */
UCLASS()
class BORNTOENDURE_API APetCompanionCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APetCompanionCharacter();

protected:
	virtual void BeginPlay() override;

public:

	/** * @brief AI Controller가 타겟 위치를 넘겨줄 때 호출하며 PetCombatComponent의 공격 함수 호출을 위임한 형태 */
	void OnAttack(const FVector& TargetLocation);

	/** * @brief PetManagerComponent가 스폰 직후 오너를 설정할 때 호출 */
	void SetFollowOwner(APawn* PlayerPawn);

private:

	// 컴포넌트 캐싱
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pet|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPetStatComponent> PetStatComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pet|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPetItemComponent> PetItemComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pet|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPetCombatComponent> PetCombatComp;

	/**
	 * @brief 적 탐지 충돌체 컴포넌트로 Pet의 공격 범위를 결정함
	 * @note 현재는 사용할 수 있으나 추후에는 다른 방식으로 변경할 예정
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pet|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> DetectionSphere;

	/**
	 * @brief Pet의 기본 데이터 에셋으로 각종 기본 스탯, 수치를 초기화한다
	 * @note PetBaseDataAsset에서 확장된 DataAsset으로 에디터 Detail에서 적용하여 사용함
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pet|Data", meta = (AllowPrivateAccess = "true", AllowedTypes = "PetBaseDataAsset"))
	TObjectPtr<UPetBaseDataAsset> PetBaseData;

	/** 
	* @brief PetBaseData를 읽어 각 컴포넌트를 초기화한다 
	* - StatComponent로 PetBaseData의 Stat을 전달하여 초기화
	*/
	void InitializeFromDataAsset();

	/** 
	* @brief StatComponent와 CombatComponent를 연결한다 
	* - 각종 Component의 Delegate Bind 등을 한 곳에서 처리하여 컴포넌트 간의 의존성을 줄이고 유지보수성을 높임
	*/
	void BindComponentDelegates();


public:

	// Getter 함수
	UPetStatComponent* GetStatComponent() const { return PetStatComp; }
	UPetItemComponent* GetItemComponent() const { return PetItemComp; }
	UPetCombatComponent* GetCombatComponent() const { return PetCombatComp; }
	UBehaviorTree* GetBehaviorTree() const;

	/**
	 * @brief BTService_PetRadar에서 Pet의 공격 범위를 가져가 사용한다
	 * @return Pet의 공격 범위
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	float GetAttackRange() const { return 500.0f; }

	// 디버그 윈도우에서 Pet 이름 표시용 Getter
	FName GetPetName() const;
};
