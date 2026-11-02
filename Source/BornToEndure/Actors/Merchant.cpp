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

