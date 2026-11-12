/**
* @file PetItemSlotWidget.h
* @brief 아이템 아이콘과 Hover·Click 이벤트를 제공하는 공용 슬롯입니다.
*/
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/StreamableManager.h"
#include "Data/DataTableRow/ItemDataRow.h"

#include "PetItemSlotWidget.generated.h"

class UButton;
class UImage;
class UTexture2D;


UCLASS()
class BORNTOENDURE_API UPetItemSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemSlotHovered, const FItemDataHandle&, ItemDataHandle, bool, bIsHovered);
	FOnItemSlotHovered OnItemSlotHovered;

	/** 아이콘 클릭 시 아이템 핸들을 전달합니다. Entry가 초기화 시 구독합니다. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemSlotClicked, const FItemDataHandle&, ItemDataHandle);
	FOnItemSlotClicked OnItemSlotClicked;

	void SetItemData(const FItemDataHandle& InItemDataHandle);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

private:

	FItemDataHandle CachedItemDataHandle;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ItemButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemImage;

	TSharedPtr<FStreamableHandle> TextureLoadingHandle;
	void SetPetIconImage(TSoftObjectPtr<UTexture2D> PetIcon);
	void OnIconsLoaded(TSoftObjectPtr<UTexture2D> LoadedIcons);

	UFUNCTION()
	void OnItemButtonClicked();

	UFUNCTION()
	void OnItemButtonHovered();
	UFUNCTION()
	void OnItemButtonUnhovered();
};
