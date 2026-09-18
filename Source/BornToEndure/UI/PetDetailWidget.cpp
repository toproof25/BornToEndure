#include "UI/PetDetailWidget.h"

#include "UI/PetItemWidget.h"
#include "UI/PetStatWidget.h"

#include "Character/Pet/PetCompanionCharacter.h"

void UPetDetailWidget::NativeOnInitialized()
{
}

void UPetDetailWidget::UpdatePetDetail(APetCompanionCharacter* InPet)
{
	if (!InPet) return;

	if (PetStatWidget) PetStatWidget->UpdatePetStats(InPet->GetFinalStats());
	if (PetItemWidget) PetItemWidget->UpdateOwnedItems(InPet->GetOwnedItemRowHandles());
}
