// Fill out your copyright notice in the Description page of Project Settings.

#include "StatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UStatComponent::UStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UStatComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UStatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


void UStatComponent::RecalculateSpeed(UCharacterMovementComponent* MoveComponent, bool isSprint)
{
	float FinalSpeed = BaseWalkSpeed * ItemSpeedBonus * (isSprint ? SprintMultiplier : 1.0f);
	MoveComponent->MaxWalkSpeed = FinalSpeed;
}