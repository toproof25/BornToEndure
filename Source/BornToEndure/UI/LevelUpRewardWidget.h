#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/GameTypes.h"
#include "Data/DataTableRow/ItemDataRow.h"
#include "LevelUpRewardWidget.generated.h"

class UImage;
class UButton;
class UTextBlock;
class UHorizontalBox;
class UPetEntryWidget;
class UItemEntryWidget;
class APetCompanionCharacter;


// Pet과 Item 선택 시 포인터를 전달하기 위한 Delegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPetSelected, APetCompanionCharacter*, SelectedPet);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemSelected, const FItemDataHandle&, SelectedItem);
//FOnItemSelected OnItemSelectedDelegate;


UCLASS()
class BORNTOENDURE_API ULevelUpRewardWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(meta = (BindWidget))
	UImage* RewardBackgroundImage;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DescriptionText;

	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;

	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* PetListBox;

	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* ItemListBox;

	UPROPERTY(EditDefaultsOnly, Category = "UI Classes")
	TSubclassOf<UPetEntryWidget> PetEntryWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI Classes")
	TSubclassOf<UItemEntryWidget> ItemEntryWidgetClass;

public:

	virtual void NativeOnInitialized() override;

	void InitializeWithLevelUpData(const FLevelUpDataBundle& InLevelUpData);

	UFUNCTION()
	void ExitButtonClicked();
	
	UFUNCTION()
	void HandlePetSelected(APetCompanionCharacter* InSelectedPet);
	UFUNCTION()
	void HandleItemSelected(const FItemDataHandle& InSelectedItem);

	/**
	 * @brief PetWidget에서 각 펫이 선택될 때 호출되는 함수로, 선택된 펫 위젯을 직관적으로 표시하기 위해 사용됨
	 * @param InSelectedPetWidget 선택된 UPetEntryWidget 포인터
	 */
	void OnSelectedPetWidget(UPetEntryWidget* InSelectedPetWidget);

	/**
	 * @brief ItemWidget에서 각 아이템이 선택될 때 호출되는 함수로, 선택된 아이템 위젯을 직관적으로 표시하기 위해 사용됨
	 * @param InSelectedItemWidget 선택된 UItemEntryWidget 포인터
	 */
	void OnSelectedItemWidget(UItemEntryWidget* InSelectedItemWidget);

private:

	UPROPERTY()
	APetCompanionCharacter* SelectedPet;
	UPROPERTY()
	FItemDataHandle SelectedItem;

	/**
	 * @brief 현재 선택된 아이템 위젯을 캐싱
	 */
	UItemEntryWidget* SelectedItemWidget;

	/**
	 * @brief 현재 선택된 펫 위젯을 캐싱
	 */
	UPetEntryWidget* SelectedPetWidget;
};
