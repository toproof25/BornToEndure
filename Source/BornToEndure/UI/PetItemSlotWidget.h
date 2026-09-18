/**
* @file PetItemSlotWidget.h
* @brief PetItemWidget 내부에서 Grid 내부에 Item 하나를 표시하는 위젯
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

	void SetItemData(const FItemDataHandle& InItemDataHandle);

protected:
	virtual void NativeOnInitialized() override;

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
	void OnItemButtonHovered();
	UFUNCTION()
	void OnItemButtonUnhovered();
};
