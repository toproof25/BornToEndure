/**
* @file CombatPlayerState.h
* @brief 플레이어가 전투를 하며 처치한 적과 데미지 통계를 집계하기 위한 PlayerState 클래스
* @data 2025-04-28
* @author toproof
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Data/GameTypes.h"
#include "CombatPlayerState.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCombatPlayerState, Log, All);

/**
 * @brief 전투 중 발생하는 모든 데미지 통계를 기록하는 PlayerState 클래스
 */
UCLASS()
class BORNTOENDURE_API ACombatPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:

	/**
	 * @brief Pet별로 적에게 준 모든 데미지를 저장하는 TMap
	 * @Key : Pet의 이름
	 * @Value : 해당 Pet이 적에게 준 총 데미지
	 * @todo
	 * - 추후 Pet이름이 아닌 Pet의 고유 ID로 변경하는 것을 고려할 수 있음 (FGuid 또는 int32)
	 */
	TMap<FName, float> PetDamageStats;

	/**
	 * @brief 데미지 통계를 업데이트하는 함수
	 * @param KillerPetName 적을 처치한 Pet의 이름
	 * @param PetDamageMap Pet별로 적에게 준 데미지를 담은 TMap
	 * @param TotalDamageReceiced Pet이 적에게 준 총 데미지 (PetDamageMap의 모든 Value의 합과 같음)
	 */
	void UpdateDamageStats(FName KillerPetName, const FPetDamageMap& PetDamageMap, float TotalDamageReceiced);

	// 디버그용으로 ImGui에서 가져가 볼 수 있도록 Getter 함수 추가
	const TMap<FName, float>& GetPetDamageStats() const { return PetDamageStats; }

};
