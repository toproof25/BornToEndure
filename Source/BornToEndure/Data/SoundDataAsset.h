// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SoundDataAsset.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class BORNTOENDURE_API USoundDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:

    // 실제 사운드 에셋 (SoundWave, SoundCue 등)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
    USoundBase* Sound;

    // 사운드 볼륨 배율 (0~1 기본 1) - 전체 음량 제어
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VolumeMultiplier = 1.0f;

    // 피치 배율 (0.5~2 기본 1) - 재생속도 및 음높이 조절
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound", meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float PitchMultiplier = 1.0f;

    // 저역 필터 컷오프 주파수 (Hz, 20k 이상은 필터 없음)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound", meta = (ClampMin = "0.0"))
    float LowPassFilterFrequency = 20000.0f;

    // 사운드 감쇠 설정 (거리별 볼륨, 공간화 등)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
    USoundAttenuation* AttenuationSettings;

    // 최대 동시 재생 수, 음성 경합 제어
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
    USoundConcurrency* ConcurrencySettings;

    // 사운드 우선순위 (높을수록 중요)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound", meta = (ClampMin = "0"))
    int32 Priority = 0;

    // 사운드 클래스 (믹싱, 볼륨 그룹화 등)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
    USoundClass* SoundClass;

	// Asset Manager 가 이 에셋을 인식할 때 쓰는 타입 이름
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("SoundDataAsset", GetFName());
	}
};
