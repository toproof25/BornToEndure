
#include "UI/PetEntryWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "UI/LevelUpRewardWidget.h"
#include "Character/Pet/PetCompanionCharacter.h"

void UPetEntryWidget::InitializeWithPetData(TObjectPtr<APetCompanionCharacter> InPet)
{
	FName PetName = InPet ? InPet->GetPetName() : NAME_None;
	FText DisplayName = FText::FromName(PetName);
	PetNameText->SetText(DisplayName);
	SelectedPet = InPet;

	// 버튼 클릭 시 OnPetSelectButtonClicked 함수가 호출
	PetSelectButton->OnClicked.AddDynamic(this, &UPetEntryWidget::OnPetSelectButtonClicked);
}

void UPetEntryWidget::OnPetSelectButtonClicked()
{
	OnPetSelectedDelegate.Broadcast(SelectedPet);
}

