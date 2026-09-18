/**
* @file PetItemTooltipWidget.h
* @brief PetItemWidget 내부에서 Item을 Hover할 때 Pet의 아이템 정보를 표시하는 위젯
*/
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/DataTableRow/ItemDataRow.h"

#include "PetItemTooltipWidget.generated.h"


class UTextBlock;

UCLASS()
class BORNTOENDURE_API UPetItemTooltipWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	void UpdateTooltip(const FItemDataHandle& ItemDataHandle);

protected:


private:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemDescriptionText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemEffectText;

};
