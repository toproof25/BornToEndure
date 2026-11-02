// Fill out your copyright notice in the Description page of Project Settings.


/**
 * @file PetItemSlotWidget.cpp
 * @brief 아이템 아이콘과 Hover·Click 이벤트를 제공하는 공용 슬롯입니다.
 */

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
		ItemButton->OnClicked.AddDynamic(this, &UPetItemSlotWidget::OnItemButtonClicked);
		ItemButton->OnHovered.AddDynamic(this, &UPetItemSlotWidget::OnItemButtonHovered);
		ItemButton->OnUnhovered.AddDynamic(this, &UPetItemSlotWidget::OnItemButtonUnhovered);
	}
}

void UPetItemSlotWidget::SetPetIconImage(TSoftObjectPtr<UTexture2D> PetIcon)
{
	if (TextureLoadingHandle.IsValid())
	{
		TextureLoadingHandle->CancelHandle();
		TextureLoadingHandle.Reset();
	}

	if (!IsValid(ItemImage))
	{
		UE_LOG(LogTemp, Warning, TEXT("[UPetItemSlotWidget] 아이콘 표시 위젯이 없습니다."));
		return;
	}

	ItemImage->SetBrushFromTexture(nullptr);
	if (PetIcon.IsNull())
	{
		return;
	}

	if (PetIcon.IsValid())
	{
		ItemImage->SetBrushFromTexture(PetIcon.Get());
		return;
	}

	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	TextureLoadingHandle = Streamable.RequestAsyncLoad(
		PetIcon.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &UPetItemSlotWidget::OnIconsLoaded, PetIcon)
	);
}

void UPetItemSlotWidget::OnIconsLoaded(TSoftObjectPtr<UTexture2D> LoadedIcons)
{
	UTexture2D* Icon = LoadedIcons.Get();
	if (IsValid(Icon) && IsValid(ItemImage))
	{
		ItemImage->SetBrushFromTexture(Icon);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[UPetItemSlotWidget] 아이콘 표시 실패: %s"), *LoadedIcons.ToString());
	}

	if (TextureLoadingHandle.IsValid())
	{
		TextureLoadingHandle.Reset();
	}
}

void UPetItemSlotWidget::OnItemButtonHovered()
{
	OnItemSlotHovered.Broadcast(CachedItemDataHandle, true);
}

void UPetItemSlotWidget::OnItemButtonUnhovered()
{
	OnItemSlotHovered.Broadcast(CachedItemDataHandle, false);
}

void UPetItemSlotWidget::NativeDestruct()
{
	if (TextureLoadingHandle.IsValid())
	{
		TextureLoadingHandle->CancelHandle();
		TextureLoadingHandle.Reset();
	}

	Super::NativeDestruct();
}

void UPetItemSlotWidget::OnItemButtonClicked()
{
	OnItemSlotClicked.Broadcast(CachedItemDataHandle);
}
