// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/PetManager.h"
#include "Character/Pet/PetCompanionCharacter.h"
#include "Engine/World.h"


UPetManager::UPetManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPetManager::BeginPlay()
{
	Super::BeginPlay();
}


void UPetManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

