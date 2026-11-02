/**
 * @file ItemStoreEntryWidget.h
 * @brief 상점 상품의 아이콘·가격·구매 상태를 표시하고 사용자 선택을 전달하는 위젯입니다.
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/DataTableRow/ItemDataRow.h"
#include "ItemStoreEntryWidget.generated.h"

class UPetItemSlotWidget;
class UTextBlock;
class UButton;
class UBorder;

/**
 * 구매할 슬롯 인덱스를 전달합니다.
 * @note HandlePurchase에서 방송하며 상점 DisplayOffers에서 구독하고 ClearEntries에서 해제합니다.
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FStorePurchaseRequested, int32);

/**
 * 슬롯 인덱스·아이템 핸들·Hover 여부를 전달하며 Click은 true로 전달합니다.
 * @note HandleHover와 HandleItemClicked에서 방송하며 상점 DisplayOffers에서 구독하고 ClearEntries에서 해제합니다.
 */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FStoreItemHovered, int32, const FItemDataHandle&, bool);

UCLASS()
class BORNTOENDURE_API UItemStoreEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 상품 표시 데이터를 설정하고 품절 상태를 해제합니다.
	 * @param InIndex 컨트롤러 상품 목록의 인덱스입니다.
	 * @param Handle 표시할 아이템 핸들입니다.
	 * @param InPrice 골드 단위 가격입니다.
	 */
	void InitializeEntry(int32 InIndex, const FItemDataHandle& Handle, int32 InPrice);

	/**
	 * 잔액과 품절 상태에 따라 구매 버튼을 갱신합니다.
	 * @param Gold 현재 골드 잔액입니다.
	 * @note 표시 판단이며 최종 구매 검증은 컨트롤러가 수행합니다.
	 */
	void UpdateAvailability(int32 Gold);

	/**
	 * 상품을 품절로 표시하고 구매 버튼을 비활성화합니다.
	 */
	void MarkSold();

	/** 선택 여부에 따라 아이콘 테두리를 갱신합니다. */
	void SetSelectedVisual(bool bSelected);

	FStorePurchaseRequested OnPurchaseRequested;

	FStoreItemHovered OnItemHovered;

protected:
	virtual void NativeOnInitialized() override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> SelectionBorder;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPetItemSlotWidget> ItemSlotWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> PriceText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> PurchaseButton;

	/** 컨트롤러 상품 배열과 대응하는 슬롯 인덱스입니다. */
	int32 Index = INDEX_NONE;
	int32 Price = 0;
	bool bSold = false;

	UFUNCTION()
	void HandlePurchase();

	UFUNCTION()
	void HandleItemClicked(const FItemDataHandle& Handle);

	UFUNCTION()
	void HandleHover(const FItemDataHandle& Handle, bool bHovered);
};
