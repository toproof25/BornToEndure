// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "EnhancedInputSubsystems.h"

#include "DefaultPlayerController.generated.h"

class UInputMappingContext;
class UPlayerHUDWidget;
class ULevelUpRewardWidget;
class UPlayerExperienceComponent;

UCLASS()
class BORNTOENDURE_API ADefaultPlayerController : public APlayerController
{
	GENERATED_BODY()


protected:

	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** * @brief Input Mapping Context를 편집기에서 할당 가능하도록 선언 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UI, meta = (AllowedClasses = "PlayerHUDWidget"))
	TSubclassOf<UPlayerHUDWidget> PlayerHUDWidgetClass;

	UPROPERTY()
	TObjectPtr<UPlayerHUDWidget> PlayerHUDWidgetInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UI)
	TSubclassOf<ULevelUpRewardWidget> LevelUpWidgetClass;

private:

	TObjectPtr<UPlayerExperienceComponent> PlayerExpComp;

	void SetUpDelegates();
	void SetUpPlayerInputMode();
	void SetUpPlayerHUDWidget();

	void HandlePlayerDeath();

	UFUNCTION()
	void LevelUpHandler(int32 NewLevel);
};
