// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"



DECLARE_LOG_CATEGORY_EXTERN(LogEffectSubsystem, Log, All);

#include "EffectSubsystem.generated.h"




class UParticleSystem;

UCLASS()
class BORNTOENDURE_API UEffectSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// 월드 서브시스템 초기화 시 호출 (BeginPlay와 비슷한 느낌)
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	UFUNCTION()
	void SpawnSoundAtLocation(FString SoundName, FVector SoundSpawnLocation);


	UFUNCTION()
	void OnSoundLoaded(FPrimaryAssetId LoadedAssetId, FVector Location);
};
