#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PetCompanionCharacter.generated.h"

class USphereComponent;

UCLASS()
class BORNTOENDURE_API APetCompanionCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APetCompanionCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;



public:

	// 추후 적 탐지 시스템을 위해 남겨두는 구형 충돌체 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> DetectionSphere;
};
