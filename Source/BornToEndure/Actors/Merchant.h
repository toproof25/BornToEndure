/**
* @file Merchant.h
* @brief 플레이어와 상호작용해 상점을 여는 상인 액터를 선언합니다.
* - IInteractable 인터페이스를 바탕으로 플레이어와 상호작용
* - 플레이어 컨트롤러를 통해 상점 오픈 함수를 직접 호출
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/Interactable.h"

#include "Merchant.generated.h"

UCLASS()
class BORNTOENDURE_API AMerchant : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	AMerchant();

protected:
	virtual void BeginPlay() override;
	virtual void Interact_Implementation(class APlayerCharacter* InstigatorCharacter, class ADefaultPlayerController* PlayerController) override;


public:



};
