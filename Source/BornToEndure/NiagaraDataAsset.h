// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

class UNiagaraSystem;
#include "NiagaraDataAsset.generated.h"


UCLASS()
class BORNTOENDURE_API UNiagaraDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara")
	TObjectPtr<UNiagaraSystem> Niagara;

	// Asset Manager 가 이 에셋을 인식할 때 쓰는 타입 이름
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("NiagaraDataAsset", GetFName());
	}
};
