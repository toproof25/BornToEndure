
#include "UI/PetEntryWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "UI/LevelUpRewardWidget.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Character/Pet/PetCompanionCharacter.h"

void UPetEntryWidget::InitializeWithPetData(ULevelUpRewardWidget* InParentWidget, TObjectPtr<APetCompanionCharacter> InPet)
{
	ParentWidget = InParentWidget;

	FName PetName = InPet ? InPet->GetPetName() : NAME_None;
	FText DisplayName = FText::FromName(PetName);
	PetNameText->SetText(DisplayName);
	SelectedPet = InPet;

	// 이미지 비동기 로딩
	TSoftObjectPtr<UTexture2D> PetIcon = InPet->GetIcon();
	if (PetIcon.IsNull())
	{
		// 추후 배경색을 빨강으로 혹은 X 표시 이미지로 적용
		//PetIconImage->SetBrushFromTexture();
	}
	else if (PetIcon.IsValid())
	{
		//OnIconsLoaded(PetIcon);
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
			FStreamableDelegate::CreateUObject(this, &UPetEntryWidget::OnIconsLoaded, PetIcon)
		);
	}

	

	// 버튼 클릭 시 OnPetSelectButtonClicked 함수가 호출
	PetSelectButton->OnClicked.AddDynamic(this, &UPetEntryWidget::OnPetSelectButtonClicked);
}

void UPetEntryWidget::OnPetSelectButtonClicked()
{
	ParentWidget->OnSelectedPetWidget(this);

	OnPetSelectedDelegate.Broadcast(SelectedPet);
}

void UPetEntryWidget::SetSelectedVisual(bool bSelected)
{
	bIsSelected = bSelected;
	if (bIsSelected)
	{
		PetBoxBorder->SetBrushColor(FLinearColor::Red); 
	}
	else
	{
		PetBoxBorder->SetBrushColor(FLinearColor::White);
	}
}


void UPetEntryWidget::OnIconsLoaded(TSoftObjectPtr<UTexture2D> LoadedIcons)
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

