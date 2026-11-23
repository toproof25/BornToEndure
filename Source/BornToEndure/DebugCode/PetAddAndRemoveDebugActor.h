// Fill out your copyright notice in the Description page of Project Settings.

/**
 * @file PetAddAndRemoveDebugActor.h
 * @brief 펫 목록을 조회하고 펫 추가를 테스트하는 ImGui 디버그 액터입니다.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#if !UE_BUILD_SHIPPING
#include "ImGuiDelegates.h"
#endif
#include "PetAddAndRemoveDebugActor.generated.h"

UCLASS()
class BORNTOENDURE_API APetAddAndRemoveDebugActor : public AActor
{
	GENERATED_BODY() 
	
public:	
	// Sets default values for this actor's properties
	APetAddAndRemoveDebugActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
#if !UE_BUILD_SHIPPING
	void RenderImGui();
	FDelegateHandle ImGuiDelegateHandle;
	FString LastResult;
#endif

};
