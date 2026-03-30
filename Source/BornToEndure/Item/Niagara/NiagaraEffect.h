// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interface/Poolable.h"
#include "NiagaraEffect.generated.h"


UCLASS()
class BORNTOENDURE_API ANiagaraEffect : public AActor, public IPoolable
{
	GENERATED_BODY()
	
public:	
	ANiagaraEffect();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void ActivateActor_Implementation() override;
	virtual void DeactivateActor_Implementation() override;


};
