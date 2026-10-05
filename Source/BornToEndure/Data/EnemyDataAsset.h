/**
* @file EnemyDataAsset.h
* @brief 적마다 다른 데이터를 저장하는 DataAsset 클래스 정의
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnemyDataAsset.generated.h"

class USkeletalMesh;
class UMaterial;

UCLASS()
class BORNTOENDURE_API UEnemyDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	// 적이 사용하는 Mesh, Hit Sound, HitNiagara 등의 데이터를 정의
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<USkeletalMesh> EnemyMesh;

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UMaterial> EnemyMaterial;

	UPROPERTY(EditAnywhere)
	FPrimaryAssetId HitEnemySoundId;

	UPROPERTY(EditAnywhere)
	FPrimaryAssetId HitEnemyNiagaraId;


	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(FPrimaryAssetType("EnemyData"), GetFName());
	}

};
