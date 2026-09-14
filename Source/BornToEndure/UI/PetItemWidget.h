/**
* @file PetItemWidget.h
* @brief Pet에 따라 소유하고 있는 Pet의 아이템 정보를 표시하는 위젯
* - PetDetailWidget에서 Pet의 참조를 받아서 데이터를 업데이트함
*/
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/DataTableRow/ItemDataRow.h"

#include "PetItemWidget.generated.h"

class UPetItemSlotWidget;
class UPetItemTooltipWidget;
class UUniformGridPanel;

UCLASS()
class BORNTOENDURE_API UPetItemWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	void UpdateOwnedItems(TArray<FItemDataHandle> InOwnedItemRowHandles);

protected:

	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

private:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPetItemSlotWidget> PetWeaponSlotWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPetItemTooltipWidget> PetItemTooltipWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> ItemGrid;

	UPROPERTY(EditDefaultsOnly, Category = "PetItemWidget")
	TSubclassOf<UPetItemSlotWidget> PetItemSlotWidgetClass;


	UFUNCTION()
	void HandleItemSlotHovered(const FItemDataHandle& ItemDataHandle, bool bIsHovered);
};
