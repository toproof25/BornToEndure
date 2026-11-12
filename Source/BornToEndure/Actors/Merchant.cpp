/**
 * @file Merchant.cpp
 * @brief 플레이어의 상인 상호작용을 컨트롤러에 전달해 상점을 엽니다.
 */

#include "Merchant.h"
#include "Interface/Interactable.h"
#include "Character/Player/PlayerCharacter.h"
#include "Core/DefaultPlayerController.h"

AMerchant::AMerchant()
{
	PrimaryActorTick.bCanEverTick = false;

}

void AMerchant::BeginPlay()
{
	Super::BeginPlay();

}

void AMerchant::Interact_Implementation(APlayerCharacter* InstigatorCharacter, ADefaultPlayerController* PlayerController)
{
	if (!InstigatorCharacter || !PlayerController) return;

	PlayerController->HandleMerchantInteraction();
}
