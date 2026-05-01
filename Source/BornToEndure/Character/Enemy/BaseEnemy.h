// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Delegates/Delegate.h"
#include "BaseEnemy.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBaseEnemy, Log, All);

class UStaticMeshComponent;
class UCapsuleComponent;

// Delegate ¼±¾ð
//DECLARE_DELEGATE_TwoParams(FOnEnemyHitSound, FName, FVector);
//DECLARE_DELEGATE_TwoParams(FOnEnemyHitNiagara, FName, FVector);

UCLASS()
class BORNTOENDURE_API ABaseEnemy : public APawn
{
	GENERATED_BODY()

public:
	ABaseEnemy();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	TObjectPtr<UStaticMeshComponent> EnemyMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	TObjectPtr<UCapsuleComponent> EnemyColision;

	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser
	);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy | Stat")
	float MaxHealth = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy | Stat")
	float CurrentHealth = 200.0f;



private:
	FOnEnemyHitSound OnEnemyHitSound;
	FOnEnemyHitNiagara OnEnemyHitNiagara;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy | HitEffects | Sound", meta = (AllowedTypes = "SoundDataAsset"))
	FPrimaryAssetId HitEnemySoundId;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy | HitEffects | Niagara", meta = (AllowedTypes = "NiagaraDataAsset"))
	FPrimaryAssetId HitEnemyNiagaraId;

};
