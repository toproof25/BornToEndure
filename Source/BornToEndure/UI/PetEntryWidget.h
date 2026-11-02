/**
* @file PetEntryWidget.h
* @brief Pet 아이콘과 선택 이벤트를 제공하는 공용 항목입니다.
* - Pet의 이름과 선택 버튼을 포함
*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/LevelUpRewardWidget.h"
#include "Engine/StreamableManager.h"

#include "PetEntryWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPetSelected, APetCompanionCharacter*, SelectedPet);

class UButton;
class UTextBlock;
class APetCompanionCharacter;
class UBorder;
class UImage;
class ULevelUpRewardWidget;

UCLASS()
class BORNTOENDURE_API UPetEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UBorder* PetBoxBorder;

	UPROPERTY(meta = (BindWidget))
	UImage* PetIconImage;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PetNameText;

	UPROPERTY(meta = (BindWidget))
	UButton* PetSelectButton;

	UPROPERTY()
	APetCompanionCharacter* SelectedPet = nullptr;

	/** 클릭 시 Pet을 방송합니다. 상점은 DisplayPets에서 구독하고 ClearPets에서 해제합니다. */
	FOnPetSelected OnPetSelectedDelegate;

	/**
	 * Pet 표시를 초기화합니다.
	 * @param InParentWidget 레벨업 부모. 상점은 nullptr로 전달하고 Delegate로 선택을 처리합니다.
	 * @param InPet 표시할 유효 Pet. 유효하지 않거나 필수 위젯이 없으면 초기화하지 않습니다.
	 */
	void InitializeWithPetData(ULevelUpRewardWidget* InParentWidget, TObjectPtr<APetCompanionCharacter> InPet);

	UFUNCTION()
	void OnPetSelectButtonClicked();

	/**
	 * @brief Pet의 선택 상태에 따라 테두리를 표시합니다.
	 * @param bSelected 선택 유무를 true/false로 전달
	 */
	void SetSelectedVisual(bool bSelected);

protected:
	virtual void NativeDestruct() override;

private:

	/** * @brief 진행 중 아이콘 요청의 수명을 관리하는 핸들입니다. */
	TSharedPtr<FStreamableHandle> TextureLoadingHandle;

	/** * @brief Pet 선택 표시 상태입니다. */
	bool bIsSelected = false;

	/** * @brief 선택 사항인 레벨업 부모입니다. 상점에서는 nullptr입니다. */
	UPROPERTY()
	ULevelUpRewardWidget* ParentWidget = nullptr;


	void SetPetIconImage(TSoftObjectPtr<UTexture2D> InTexture);

	/**
	 * @brief 이미지 로드 후 호출되는 콜백 함수
	 * @param LoadedIcons 메모리에 로드가 완료된 이미지의 경로 (TSoftObjectPtr<UTexture2D>)
	 */
	void OnIconsLoaded(TSoftObjectPtr<UTexture2D> LoadedIcons);
};
