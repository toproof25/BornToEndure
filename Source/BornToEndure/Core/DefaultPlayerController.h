// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "EnhancedInputSubsystems.h"

#include "DefaultPlayerController.generated.h"

class UInputMappingContext;

UCLASS()
class BORNTOENDURE_API ADefaultPlayerController : public APlayerController
{
	GENERATED_BODY()
	

protected:

	virtual void BeginPlay() override;

	/** * @brief Input Mapping Context를 편집기에서 할당 가능하도록 선언 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
};
