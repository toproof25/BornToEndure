// Fill out your copyright notice in the Description page of Project Settings.

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
	FImGuiDelegateHandle ImGuiDelegateHandle;
	FString LastResult;
#endif

};
