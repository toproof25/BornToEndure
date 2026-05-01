#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BaseEnemyAIController.generated.h"

UCLASS()
class BORNTOENDURE_API ABaseEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
    virtual void OnPossess(APawn* InPawn) override;

    void SetTargetPlayer(APawn* Player);
};
