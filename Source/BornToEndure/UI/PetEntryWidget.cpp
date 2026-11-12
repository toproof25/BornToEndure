/**
 * @file PetEntryWidget.cpp
 * @brief Pet 아이콘과 선택 이벤트를 제공하는 공용 항목입니다.
 */

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
	SelectedPet = nullptr;
	if (!IsValid(InPet.Get()) || !PetNameText || !PetIconImage || !PetSelectButton || !PetBoxBorder)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UPetEntryWidget] Pet 또는 필수 표시 위젯이 없습니다."));
		return;
	}

	SelectedPet = InPet;
	PetNameText->SetText(InPet->GetPetName());
	SetSelectedVisual(false);
	SetPetIconImage(InPet->GetIcon());

	PetSelectButton->OnClicked.AddUniqueDynamic(this, &UPetEntryWidget::OnPetSelectButtonClicked);
}

void UPetEntryWidget::OnPetSelectButtonClicked()
{
	if (!IsValid(SelectedPet))
	{
		return;
	}

	if (IsValid(ParentWidget))
	{
		ParentWidget->OnSelectedPetWidget(this);
	}

	OnPetSelectedDelegate.Broadcast(SelectedPet);
}

void UPetEntryWidget::SetSelectedVisual(bool bSelected)
{
	bIsSelected = bSelected;
	if (PetBoxBorder)
	{
		PetBoxBorder->SetBrushColor(bIsSelected ? FLinearColor::Red : FLinearColor::White);
	}
}

void UPetEntryWidget::SetPetIconImage(TSoftObjectPtr<UTexture2D> PetIcon)
{
	if (TextureLoadingHandle.IsValid())
	{
		TextureLoadingHandle->CancelHandle();
		TextureLoadingHandle.Reset();
	}

	if (!IsValid(PetIconImage))
	{
		UE_LOG(LogTemp, Warning, TEXT("[UPetEntryWidget] 아이콘 표시 위젯이 없습니다."));
		return;
	}

	PetIconImage->SetBrushFromTexture(nullptr);
	if (PetIcon.IsNull())
	{
		return;
	}

	if (PetIcon.IsValid())
	{
		PetIconImage->SetBrushFromTexture(PetIcon.Get());
		return;
	}

	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	TextureLoadingHandle = Streamable.RequestAsyncLoad(
		PetIcon.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &UPetEntryWidget::OnIconsLoaded, PetIcon)
	);
}

void UPetEntryWidget::OnIconsLoaded(TSoftObjectPtr<UTexture2D> LoadedIcons)
{
	UTexture2D* Icon = LoadedIcons.Get();
	if (IsValid(Icon) && IsValid(PetIconImage))
	{
		PetIconImage->SetBrushFromTexture(Icon);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[UPetEntryWidget] 아이콘 표시 실패: %s"), *LoadedIcons.ToString());
	}

	if (TextureLoadingHandle.IsValid())
	{
		TextureLoadingHandle.Reset();
	}
}

void UPetEntryWidget::NativeDestruct()
{
	if (TextureLoadingHandle.IsValid())
	{
		TextureLoadingHandle->CancelHandle();
		TextureLoadingHandle.Reset();
	}

	Super::NativeDestruct();
}
