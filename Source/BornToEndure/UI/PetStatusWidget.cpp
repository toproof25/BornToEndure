#include "UI/PetStatusWidget.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"

#include "Character/Pet/PetCompanionCharacter.h"

#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"

void UPetStatusWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (PetSelectButton)
	{
		PetSelectButton->OnClicked.AddDynamic(this, &UPetStatusWidget::HandlePetSelectButtonClicked);
	}
}

void UPetStatusWidget::InitializeWithPetData(APetCompanionCharacter* InPet)
{
	if (!InPet)
	{
		UE_LOG(LogTemp, Warning, TEXT("UPetStatusWidget: InitializeWithPetData called with null InPet."));
		return;
	}

	PetCharacter = InPet;
	if (PetNameText)
	{
		PetNameText->SetText(InPet->GetPetName());
	}
	if (PetIconImage)
	{
		SetPetIconImage(InPet->GetIcon());
	}
}


void UPetStatusWidget::HandlePetSelectButtonClicked()
{
	OnPetStatusSelected.Broadcast(PetCharacter.Get());
	UE_LOG(LogTemp, Log, TEXT("UPetStatusWidget: Pet selected: %s"), *PetCharacter->GetName());
}

void UPetStatusWidget::SetPetIconImage(TSoftObjectPtr<UTexture2D> PetIcon)
{
	if (PetIcon.IsNull())
	{
		// 추후 배경색을 빨강으로 혹은 X 표시 이미지로 적용
		//PetIconImage->SetBrushFromTexture();
	}
	else if (PetIcon.IsValid())
	{
		PetIconImage->SetBrushFromTexture(PetIcon.Get());
	}
	else if (TextureLoadingHandle.IsValid() && TextureLoadingHandle->IsActive())
	{
		// 이미 로딩 중인 경우 처리
		TextureLoadingHandle->CancelHandle();
	}
	else
	{
		FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
		TextureLoadingHandle = Streamable.RequestAsyncLoad(
			PetIcon.ToSoftObjectPath(),
			FStreamableDelegate::CreateUObject(this, &UPetStatusWidget::OnIconsLoaded, PetIcon)
		);
	}
}
void UPetStatusWidget::OnIconsLoaded(TSoftObjectPtr<UTexture2D> LoadedIcons)
{
	UTexture2D* Icon = LoadedIcons.Get();
	if (Icon)
	{
		PetIconImage->SetBrushFromTexture(Icon);
		UE_LOG(LogTemp, Log, TEXT("UPetEntryWidget: Successfully loaded icon for pet: %s"), *LoadedIcons.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UPetEntryWidget: Failed to load icon for pet: %s"), *LoadedIcons.ToString());
	}

	TextureLoadingHandle->CancelHandle();
}
