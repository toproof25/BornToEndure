/**
* @file PetEntryWidget.h
* @date 2026-06-16
* @modified 2026-06-16
* @author toproof (kmnlmn123@gmail.com)
* @brief PetEntryWidget.h 레벨업 보상 창에서 각 Pet을 나타내는 위젯 클래스
* - Pet의 이름과 선택 버튼을 포함
*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/LevelUpRewardWidget.h"
#include "Engine/StreamableManager.h"

#include "PetEntryWidget.generated.h"


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

	APetCompanionCharacter* SelectedPet;

	FOnPetSelected OnPetSelectedDelegate;

	void InitializeWithPetData(ULevelUpRewardWidget* InParentWidget, TObjectPtr<APetCompanionCharacter> InPet);

	UFUNCTION()
	void OnPetSelectButtonClicked();

	/**
	 * @brief 선택된 아이템 UI를 시각적으로 표시하기 위한 함수로, bool 값에 따라 Border의 상태가 변경됨
	 * @param bSelected 선택 유무를 true/false로 전달
	 */
	void SetSelectedVisual(bool bSelected);

protected:

	/**
	 * @brief 이미지를 비동기 로드 후 GC를 방지하기 위한 캐싱 변수
	 */
	TSharedPtr<FStreamableHandle> TextureLoadingHandle;

	/**
	 * @brief 이미지 로드 후 호출되는 콜백 함수
	 * @param LoadedIcons 메모리에 로드가 완료된 이미지의 경로 (TSoftObjectPtr<UTexture2D>)
	 */
	void OnIconsLoaded(TSoftObjectPtr<UTexture2D> LoadedIcons);

private:
	/**
	 * @brief 선택된 아이템의 시각적 효과를 나타내는 변수
	 */
	bool bIsSelected = false;

	/**
	 * @brief 부모 Widtet을 참조하기 위한 변수
	 */
	ULevelUpRewardWidget* ParentWidget;
};
