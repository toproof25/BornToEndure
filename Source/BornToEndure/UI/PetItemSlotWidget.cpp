// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PetItemSlotWidget.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

#include "Subsystem/ItemPoolSubsystem.h"

#include "Components/Button.h"
#include "Components/Image.h"


void UPetItemSlotWidget::SetItemData(const FItemDataHandle& InItemDataHandle)
{
	UWorld* World = GetWorld();
	if (!World) return;
	UItemPoolSubsystem* ItemPoolSubsystem = World->GetGameInstance()->GetSubsystem<UItemPoolSubsystem>();
	if (!ItemPoolSubsystem) return;

	const FItemDataRow* ItemDataRow = nullptr;
	if (InItemDataHandle.ItemType == EItemType::Stat)
	{
		ItemDataRow = ItemPoolSubsystem->GetStatItemDataRowByID(InItemDataHandle.ItemRowName);
	}
	else if (InItemDataHandle.ItemType == EItemType::Weapon)
	{
		ItemDataRow = ItemPoolSubsystem->GetWeaponItemDataRowByID(InItemDataHandle.ItemRowName);
	}

	if (ItemDataRow)
	{
		CachedItemDataHandle = InItemDataHandle;
		SetPetIconImage(ItemDataRow->ItemIcon);
		UE_LOG(LogTemp, Log, TEXT("[PetItemWidget] UpdateOwnedItems: ItemRowName: %s, ItemImagePath: %s"), *InItemDataHandle.ItemRowName.ToString(), *ItemDataRow->ItemIcon.ToSoftObjectPath().ToString());
	}

}

void UPetItemSlotWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (ItemButton)
	{
		//ItemButton->OnClicked.AddDynamic(this, &UPetItemSlotWidget::OnItemButtonClicked);
		ItemButton->OnHovered.AddDynamic(this, &UPetItemSlotWidget::OnItemButtonHovered);
		ItemButton->OnUnhovered.AddDynamic(this, &UPetItemSlotWidget::OnItemButtonUnhovered);
	}
}

void UPetItemSlotWidget::SetPetIconImage(TSoftObjectPtr<UTexture2D> PetIcon)
{
	if (PetIcon.IsNull())
	{
		// 추후 배경색을 빨강으로 혹은 X 표시 이미지로 적용
		//PetIconImage->SetBrushFromTexture();
	}
	else if (PetIcon.IsValid())
	{
		ItemImage->SetBrushFromTexture(PetIcon.Get());
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
			FStreamableDelegate::CreateUObject(this, &UPetItemSlotWidget::OnIconsLoaded, PetIcon)
		);
	}
}
void UPetItemSlotWidget::OnIconsLoaded(TSoftObjectPtr<UTexture2D> LoadedIcons)
{
	UTexture2D* Icon = LoadedIcons.Get();
	if (Icon)
	{
		ItemImage->SetBrushFromTexture(Icon);
		UE_LOG(LogTemp, Log, TEXT("UPetEntryWidget: Successfully loaded icon for pet: %s"), *LoadedIcons.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UPetEntryWidget: Failed to load icon for pet: %s"), *LoadedIcons.ToString());
	}

	TextureLoadingHandle->CancelHandle();
}

void UPetItemSlotWidget::OnItemButtonHovered()
{
	OnItemSlotHovered.Broadcast(CachedItemDataHandle, true);
}

void UPetItemSlotWidget::OnItemButtonUnhovered()
{
	OnItemSlotHovered.Broadcast(CachedItemDataHandle, false);
}
