// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "imgui.h"
#include "ImGuiModule.h"     
#include "ImGuiDelegates.h" 

#include "TestNiagaraPoolActor.generated.h"

class UStaticMeshComponent;
class UNiagaraDataAsset;
enum class ENCPoolMethod : uint8;

UCLASS()
class BORNTOENDURE_API ATestNiagaraPoolActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ATestNiagaraPoolActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	

	UPROPERTY(EditDefaultsOnly, Category = "Test")
	TObjectPtr<UStaticMeshComponent> StaticMesh;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Test", meta = (AllowedTypes = "NiagaraDataAsset"))
	FPrimaryAssetId NiagaraDataAssetName;

	UPROPERTY(EditAnywhere, Category = "Test")
	int32 SpawnCount = 500;

	// --- 연속 스폰 테스트(기관총)용 변수 ---
	int32 FireDurationSeconds = 5;       // N초 동안 쏠 것인가?
	int32 FireSpawnCountPerTick = 10;    // 0.1초마다 M개를 쏠 것인가? (즉 1초에 M*10개)

	int32 CurrentFireTick = 0;           // 현재 몇 번 쐈는지 카운트
	int32 MaxFireTick = 0;               // 총 쏴야 할 횟수
	FTimerHandle FireTimerHandle;        // 타이머 핸들
	ENCPoolMethod CurrentTestPoolMethod; // 현재 적용 중인 풀링 메서드

	void StartContinuousFire(ENCPoolMethod Method);
	void OnContinuousFireTick();

	FImGuiDelegateHandle ImGuiDelegateHandle;
	void LoadKoreanFontToImGui();

	UNiagaraDataAsset* GetLoadedNiagaraData();
	void LoadAssetToMemory();
	void WarmUpPool();
	void RenderImGui();
	void TestNoPooling();
	void TestWithPooling();
};
