// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BORNTOENDURE_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;



private:
	UPROPERTY()
	APawn* Owner;
	UPROPERTY()
	APlayerController* PlayerController;

	FCollisionShape SphereShape;
	FCollisionQueryParams QueryParams;

public:

	UFUNCTION(BlueprintCallable)
	bool PerformInteractionTrace(FHitResult& OutHitResult);

	UFUNCTION()
	void TestInteraction();

};
