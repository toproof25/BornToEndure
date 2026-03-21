
// Fill out your copyright notice in the Description page of Project Settings.


#include "RifleWeapon.h"

ARifleWeapon::ARifleWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ARifleWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void ARifleWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
