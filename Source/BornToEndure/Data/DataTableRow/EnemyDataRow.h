/**
* @file EnemyDataRow.h
* @date 2026-05-16
* @author toproof (kmnlmn123@gmail.com)
* @brief 적 데이터 테이블 행 구조체 정의하는 파일로 DataTable의 데이터들을 정의하기 위해 작성
*/


#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/DataAsset.h"
#include "EnemyDataRow.generated.h"


USTRUCT(BlueprintType)
struct FEnemyDataRow : public FTableRowBase
{
	GENERATED_BODY()


public:

	// 적에 대한 고유 ID, Name, 설명, 체력, 공격력, 방어력, 이동 속도, 아이콘, DataAsset(사운드, 나이아가라, 매쉬 등)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	FName EnemyID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	FText EnemyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	int32 SpawnLevel;

	/**
	 * @brief 적이 사용하는 Mesh, Sound, Niagara 등의 데이터를 담고 있는 DataAsset에 대한 SoftObjectPtr
	 * - DataTable은 로드할 때 TObjectPtr을 사용하면 모두 로드한 상태가 됨 (성능 저하 및 메모리 낭비)
	 * - TSoftObjectPtr을 사용하면 실제로 필요한 시점에 로드할 수 있다
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	TSoftObjectPtr<UDataAsset> EnemyDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	float AttackPower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	float DefensePower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	float MovementSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	float RewardExp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	int32 RewardGold;

};

