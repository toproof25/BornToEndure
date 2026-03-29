// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Poolable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPoolable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class BORNTOENDURE_API IPoolable
{
	GENERATED_BODY()

public:

	// Object Pool 요청 시 활성화 함수 
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void ActivateActor();

	// Object Pool 요청 시 비활성화 함수 
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void DeactivateActor();

};
