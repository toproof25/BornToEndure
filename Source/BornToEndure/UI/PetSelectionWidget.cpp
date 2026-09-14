#include "UI/PetSelectionWidget.h"

#include "Character/Pet/PetCompanionCharacter.h"
#include "Component/PetManagerComponent.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"

#include "UI/PetStatusWidget.h"

#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"

void UPetSelectionWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UPetSelectionWidget::InitializeWidget(const UPetManagerComponent* PetManagerComponent)
{
	if (!PetVerticalBox)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UPetSelectionWidget] InitializeWidget: PetVerticalBox is not bound."));
		return;
	}
	const TArray<TObjectPtr<APetCompanionCharacter>>& PetList = PetManagerComponent->GetPetList();

	PetVerticalBox->ClearChildren();
	for (const TObjectPtr<APetCompanionCharacter>& Pet : PetList)
	{
		// Pet 위젯 초기화 로직 추가
		UPetStatusWidget* PetStatusWidget = CreateWidget<UPetStatusWidget>(this, PetStatusWidgetClass);
		PetStatusWidget->InitializeWithPetData(Pet);
		PetStatusWidget->OnPetStatusSelected.AddDynamic(this, &UPetSelectionWidget::HandlePetStatusSelected);
		PetVerticalBox->AddChild(PetStatusWidget);
		//UE_LOG(LogTemp, Log, TEXT("[UPetSelectionWidget] Successfully added pet icon for pet: %s"), *Pet->GetName());
	}
}

void UPetSelectionWidget::HandlePetStatusSelected(APetCompanionCharacter* InSelectedPet)
{
	OnPetSelected.Broadcast(InSelectedPet);
	UE_LOG(LogTemp, Log, TEXT("[UPetSelectionWidget] HandlePetStatusSelected: Pet selected: %s"), *InSelectedPet->GetName());
}
