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

#include "PetEntryWidget.generated.h"


class UButton;
class UTextBlock;
class APetCompanionCharacter;

/**
 * 
 */
UCLASS()
class BORNTOENDURE_API UPetEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PetNameText;

	UPROPERTY(meta = (BindWidget))
	UButton* PetSelectButton;

	APetCompanionCharacter* SelectedPet;

	FOnPetSelected OnPetSelectedDelegate;

	void InitializeWithPetData(TObjectPtr<APetCompanionCharacter> InPet);

	UFUNCTION()
	void OnPetSelectButtonClicked();
	
};
