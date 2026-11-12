/**
 * @file ItemStoreEntryWidget.cpp
 * @brief 상점 상품 표시와 버튼 이벤트를 처리합니다.
 */

#include "UI/ItemStoreEntryWidget.h"
#include "UI/PetItemSlotWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"

void UItemStoreEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (PurchaseButton)
	{
		PurchaseButton->OnClicked.AddDynamic(this, &UItemStoreEntryWidget::HandlePurchase);
	}
	if (ItemSlotWidget)
	{
		ItemSlotWidget->OnItemSlotHovered.AddDynamic(this, &UItemStoreEntryWidget::HandleHover);
		ItemSlotWidget->OnItemSlotClicked.AddDynamic(this, &UItemStoreEntryWidget::HandleItemClicked);
	}
}

void UItemStoreEntryWidget::InitializeEntry(int32 InIndex, const FItemDataHandle& Handle, int32 InPrice)
{
	Index = InIndex;
	Price = InPrice;
	bSold = false;
	SetSelectedVisual(false);

	if (ItemSlotWidget)
	{
		ItemSlotWidget->SetItemData(Handle);
	}
	if (PriceText)
	{
		PriceText->SetText(FText::AsNumber(Price));
	}
}

void UItemStoreEntryWidget::UpdateAvailability(int32 Gold)
{
	// 품절이 아니고 유효한 가격을 지불할 수 있을 때 구매 버튼을 활성화합니다.
	if (PurchaseButton)
	{
		PurchaseButton->SetIsEnabled(!bSold && Price >= 0 && Gold >= Price);
	}
}

void UItemStoreEntryWidget::MarkSold()
{
	bSold = true;

	if (PurchaseButton)
	{
		PurchaseButton->SetIsEnabled(false);
	}
	if (PriceText)
	{
		PriceText->SetText(FText::FromString(TEXT("품절")));
	}
}

void UItemStoreEntryWidget::HandlePurchase()
{
	if (!bSold)
	{
		OnPurchaseRequested.Broadcast(Index);
	}
}

void UItemStoreEntryWidget::HandleHover(const FItemDataHandle& Handle, bool bHovered)
{
	OnItemHovered.Broadcast(Index, Handle, bHovered);
}

void UItemStoreEntryWidget::HandleItemClicked(const FItemDataHandle& Handle)
{
	OnItemHovered.Broadcast(Index, Handle, true);
}

void UItemStoreEntryWidget::SetSelectedVisual(bool bSelected)
{
	if (SelectionBorder)
	{
		SelectionBorder->SetBrushColor(bSelected ? FLinearColor::Red : FLinearColor::White);
	}
}
