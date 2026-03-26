// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "GameFramework/CharacterMovementComponent.h"

#include "StatComponent.generated.h"

class UCharacterMovementComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BORNTOENDURE_API UStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStatComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


private:

	UPROPERTY(EditAnywhere, Category = "Stat")
	float BaseWalkSpeed = 300.f;

	UPROPERTY(EditAnywhere, Category = "Stat")
	float SprintMultiplier = 1.5f; // 달릴 때는 1.5배 빨라짐

	UPROPERTY()
	bool bIsSprinting = false;

	UPROPERTY()
	float ItemSpeedBonus = 1.0f; // 아이템 효과 (예: +20%면 1.2f)

public:

	UFUNCTION()
	void RecalculateSpeed(UCharacterMovementComponent* MoveComponent, bool isSprint);

	float GetBaseWalkSpeed() const { return BaseWalkSpeed; }
	float GetSprintMultiplier() const { return SprintMultiplier; }
	bool GetIsSprinting() const { return bIsSprinting; }
	float GetItemSpeedBonus() const { return ItemSpeedBonus; }
};
