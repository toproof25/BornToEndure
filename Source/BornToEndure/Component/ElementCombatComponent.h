/**
* @file ElementCombatComponent.h
* @brief 피격 시 속성에 따른 데미지 계산하는 컴포넌트
* - 각 속성별 저항 수치를 TMap으로 초기화하여 관리
* - 파라미터로 넘어온 태그에 대한 저항 수치를 계산하여 데미지를 반환
*/
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Data/DataTableRow/EnemyDataRow.h"

#include "ElementCombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BORNTOENDURE_API UElementCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UElementCombatComponent();

	/**
	 * @brief DataTable에서 가져온 ElementResistance 정보를 기반으로 ElementResistanceMap 초기화하는 함수
	 * @param ElementResistances DataTable에 정의된 데이터
	 */
	void InitializeElementResistance(const TArray<FEnamyElementResistanceEntry>& ElementResistances);

	/**
	 * @brief 적이 가진 속성 저항력을 기본 데미지에 적용하여 반환하는 함수
	 * @param BaseDamage 속성 저항력 적용 전 데미지
	 * @param ElementTag 적용할 속성 태그
	 * @return 속성 저항력이 적용된 데미지
	 */
	float CalculateDamageWithElementResistance(float BaseDamage, const FGameplayTag& ElementTag) const;

private:

	TMap<FGameplayTag, float> ElementResistanceMap;

		
};
