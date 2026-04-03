// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/AssetManager.h" 
#include "EffectSubsystem.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogEffectSubsystem, Log, All);

USTRUCT()
struct FLoadedAsset
{
	GENERATED_BODY()

	TSharedPtr<FStreamableHandle> StreamableHandle;
	int32 Count = 0;
};

UCLASS()
class BORNTOENDURE_API UEffectSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// 월드 서브시스템 초기화 시 호출 (BeginPlay와 비슷한 느낌)
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void PreloadEffectAssets(FPrimaryAssetId PrimaryAssetId);
	void UnloadEffectAssets(FPrimaryAssetId PrimaryAssetId);

	UFUNCTION()
	void SpawnSoundAtLocation(FName SoundName, FVector SoundSpawnLocation);

	UFUNCTION()
	void OnSoundLoaded(FPrimaryAssetId LoadedAssetId, FVector Location);



	UFUNCTION()
	void SpawnNiagaraAtLocation(FName NiagaraName, FVector NiagaraSpawnLocation);

	UFUNCTION()
	void OnNiagaraLoaded(FPrimaryAssetId LoadedAssetId, FVector Location);


private:

	TMap<FPrimaryAssetId, FLoadedAsset> PreloadAsset;


public:

	/**
	 * @brief ImGui에서 로드된 에셋 카운트를 확인하기 위한 Getter
	 * @return Load된 에셋의 PrimaryAssetId와 카운트가 담긴 TMap
	 */
	const TMap<FPrimaryAssetId, FLoadedAsset>& GetPreloadAssetCounts() const { return PreloadAsset; }

};
