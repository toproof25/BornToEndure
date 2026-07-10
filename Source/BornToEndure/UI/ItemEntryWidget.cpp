#include "UI/ItemEntryWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Data/DataTableRow/ItemDataRow.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

void UItemEntryWidget::InitializeWithItemData(ULevelUpRewardWidget* InParentWidget, TObjectPtr<UObject> InItem)
{
	if (!InParentWidget) return;
	if (!InItem) return;

	this->ParentWidget = InParentWidget;

	UItemDataObject* ItemDataObject = Cast<UItemDataObject>(InItem);
	FItemDataRow* ItemData = &(ItemDataObject->ItemData);
	FItemText* ItemText = &(ItemData->ItemText);

	// 이미지 비동기 로딩
	TSoftObjectPtr<UTexture2D>& ItemIcon = ItemData->ItemIcon;
	if (ItemIcon.IsNull())
	{
		// 추후 배경색을 빨강으로 혹은 X 표시 이미지로 적용
		//ItemIconImage->SetBrushFromTexture();
	}
	else if (ItemIcon.IsValid())
	{
		//OnIconsLoaded(ItemIcon);
		ItemIconImage->SetBrushFromTexture(ItemIcon.Get());
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
			ItemIcon.ToSoftObjectPath(),
			FStreamableDelegate::CreateUObject(this, &UItemEntryWidget::OnIconsLoaded, ItemIcon)
		);
	}

	// 이름, 설명 등 설정
	ItemNameText->SetText(ItemText->Name);
	ItemDescriptionText->SetText(ItemText->Description);

	SelectedItem = InItem;

	// 버튼 클릭 시 OnItemSelectButtonClicked 함수가 호출
	ItemSelectButton->OnClicked.AddDynamic(this, &UItemEntryWidget::OnItemSelectButtonClicked);
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
