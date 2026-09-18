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

	/**
	 * @brief 무기 아이템 슬롯 위젯
	 * - 무기 아이템의 경우 항상 1개만 소유하므로 동적으로 생성하는게 아닌 미리 생성된 위젯을 사용
	 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPetItemSlotWidget> PetWeaponSlotWidget;

	/**
	 * @brief 각 ItemSlot에서 Hover된 아이템의 설명을 표시하는 위젯
	 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPetItemTooltipWidget> PetItemTooltipWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> ItemGrid;

	/**
	 * @brief 동적으로 생성되는 Stat Item 슬롯 위젯의 클래스
	 * - Stat Item은 여러 개를 소유할 수 있으므로 동적으로 생성
	 * - 생성을 위한 TSubclassOf 변수로 에디터에서 지정 가능
	 */
	UPROPERTY(EditDefaultsOnly, Category = "PetItemWidget")
	TSubclassOf<UPetItemSlotWidget> PetItemSlotWidgetClass;


	UFUNCTION()
	void HandleItemSlotHovered(const FItemDataHandle& ItemDataHandle, bool bIsHovered);
};
