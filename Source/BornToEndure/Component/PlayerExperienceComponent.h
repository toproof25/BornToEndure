/**
 * @file PlayerExperienceComponent.h
 * @brief 플레이어의 경험치와 골드를 관리하는 컴포넌트 헤더
 * @author toproof (kmnlmn123@gmail.com)
 * @date 2026-04-27
 * @details
 * - 이 파일은 플레이어의 경험치와 골드를 관리를 목적으로 구현
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/GameTypes.h"
#include "PlayerExperienceComponent.generated.h"

class ABaseEnemyCharacter;

DECLARE_LOG_CATEGORY_EXTERN(LogPlayerExperienceComponent, Log, All);


/**
 * @brief 플레이어의 경험치와 보상을 관리하는 컴포넌트	
 * @datails
 * - 적이 스폰될 때 마다 RegisterEnemyPayload를 호출하여 적의 Delegate를 구독 (외부 스포너에서 호출)
 * - 적 처지 시 경험치와 보상을 연산
 * - PlayerState로 통계 데이터 전송
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BORNTOENDURE_API UPlayerExperienceComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPlayerExperienceComponent();

protected:
	virtual void BeginPlay() override;

public:	

	/**
	 * @brief 외부에서 적 처치 시 방송되는 Delegate를 구독하기 위한 함수
	 * @param Enemy 구독하고자 하는 Enemy
	 * @details
	 * - 해당 함수는 적 스포너에서 적이 스폰될 때 마다 각 Enemy가 가진 Delegate에 구독하도록 호출된다
	 * - 각 적마다 각자의 Delegate가 존재하므로, 적이 스폰될 때마다 구독하는 방식
	 */
	void RegisterEnemyPayload(ABaseEnemyCharacter* Enemy);

	/**
	 * @brief 적 처치 시 Delegate로 호출되는 함수로 경험치와 보상을 처리한다
	 * @param Payload 적이 보낸 보상 정보 구조체
	 * @details
	 * - 경험치와 보상을 처리하는 것 이외에 PlayerState로 통계 데이터 전송도 담당한다
	 */
	UFUNCTION()
	void OnEnemyKilledHandler(const FEnemyRewardPayload& Payload);

	float CurrentXP = 0.f;;
	int32 CurrentGold = 0;

public:
	
	// 디버깅 용도
	float GetCurrentXP() const { return CurrentXP; }
	int32 GetCurrentGold() const { return CurrentGold; }


};
