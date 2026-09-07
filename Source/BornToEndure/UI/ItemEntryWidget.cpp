#include "UI/ItemEntryWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Data/DataTableRow/ItemDataRow.h"
#include "Data/DataTableRow/StatItemDataRow.h"
#include "Data/DataTableRow/WeaponItemDataRow.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Subsystem/ItemPoolSubsystem.h"

void UItemEntryWidget::InitializeWithItemData(ULevelUpRewardWidget* InParentWidget, UItemPoolSubsystem* ItemPool, const FItemDataHandle& InItem)
{
	if (!InParentWidget || !ItemPool) return;

	ParentWidget = InParentWidget;

	const FItemDataRow* ItemDataRow = nullptr;
	switch (InItem.ItemType)
	{
		case EItemType::Stat:
		{
			ItemDataRow = ItemPool->GetStatItemDataRowByID(InItem.ItemRowName);
			UE_LOG(LogTemp, Warning, TEXT("[UItemEntryWidget] LevelUpHandler: 랜덤 아이템 - Type: Stat"));
			break;
		}
		case EItemType::Weapon:
		{
			ItemDataRow = ItemPool->GetWeaponItemDataRowByID(InItem.ItemRowName);
			UE_LOG(LogTemp, Warning, TEXT("[UItemEntryWidget] LevelUpHandler: 랜덤 아이템 - Type: Weapon"));
			break;
		}
		default:
		{
			UE_LOG(LogTemp, Warning, TEXT("[UItemEntryWidget] LevelUpHandler: 랜덤 아이템 - Type: Unknown"));
			break;
		}
	}

	if (ItemDataRow != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UItemEntryWidget] LevelUpHandler: 랜덤 아이템 - Name: %s, Description: %s"), *ItemDataRow->ItemText.Name.ToString(), *ItemDataRow->ItemText.Description.ToString());

		// 이미지 비동기 로딩
		SetItemIconImage(ItemDataRow->ItemIcon);

		// 이름, 설명 등 설정
		FItemText ItemText = ItemDataRow->ItemText;
		ItemNameText->SetText(ItemText.Name);
		ItemDescriptionText->SetText(ItemText.Description);

		SelectedItem = InItem;

		// 버튼 클릭 시 OnItemSelectButtonClicked 함수가 호출
		ItemSelectButton->OnClicked.AddDynamic(this, &UItemEntryWidget::OnItemSelectButtonClicked);
	}
}


void UItemEntryWidget::OnItemSelectButtonClicked()
{
	ParentWidget->OnSelectedItemWidget(this);

	OnItemSelectedDelegate.Broadcast(SelectedItem);
}

void UItemEntryWidget::SetSelectedVisual(bool bSelected)
{
	bIsSelected = bSelected;
	if (bIsSelected)
	{
		ItemBoxBorder->SetBrushColor(FLinearColor::Yellow); // 선택된 상태일 때 노란색으로 변경
	}
	else
	{
		ItemBoxBorder->SetBrushColor(FLinearColor::White); // 선택되지 않은 상태일 때 흰색으로 변경
	}
}

void UItemEntryWidget::OnIconsLoaded(TSoftObjectPtr<UTexture2D> LoadedIcons)
{
	UTexture2D* Icon = LoadedIcons.Get();
	if (Icon)
	{
		ItemIconImage->SetBrushFromTexture(Icon);
		UE_LOG(LogTemp, Log, TEXT("UItemEntryWidget: Successfully loaded icon for item: %s"), *LoadedIcons.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UItemEntryWidget: Failed to load icon for item: %s"), *LoadedIcons.ToString());
	}

	TextureLoadingHandle->CancelHandle();
}

void UItemEntryWidget::SetItemIconImage(const TSoftObjectPtr<UTexture2D>& IconTextureSoftPtr)
{
	if (IconTextureSoftPtr.IsNull())
	{
		// 추후 배경색을 빨강으로 혹은 X 표시 이미지로 적용
		//ItemIconImage->SetBrushFromTexture();
	}
	else if (IconTextureSoftPtr.IsValid())
	{
		//OnIconsLoaded(ItemIcon);
		ItemIconImage->SetBrushFromTexture(IconTextureSoftPtr.Get());
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
			IconTextureSoftPtr.ToSoftObjectPath(),
			FStreamableDelegate::CreateUObject(this, &UItemEntryWidget::OnIconsLoaded, IconTextureSoftPtr)
		);
	}
}
