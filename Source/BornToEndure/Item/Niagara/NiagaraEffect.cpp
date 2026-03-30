// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Niagara/NiagaraEffect.h"
#include "NiagaraComponent.h"
#include "Subsystem/ObjectPoolSubsystem.h"

ANiagaraEffect::ANiagaraEffect()
{
	PrimaryActorTick.bCanEverTick = false;

}

void ANiagaraEffect::BeginPlay()
{
	Super::BeginPlay();

	//UNiagaraComponent* NiagaraComp = GetNiagaraComponent();
	//if (NiagaraComp)
	//{
	//	UWorld* world = GetWorld();
	//	check(world);
	//	UObjectPoolSubsystem* ObjectPool = world->GetSubsystem<UObjectPoolSubsystem>();
	//	check(ObjectPool);

	//	//NiagaraComp->OnSystemFinished.AddDynamic(ObjectPool, &UObjectPoolSubsystem::ReturnPoolActor);
	//}
	//
}

void ANiagaraEffect::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ANiagaraEffect::ActivateActor_Implementation()
{

}

void ANiagaraEffect::DeactivateActor_Implementation()
{

}

