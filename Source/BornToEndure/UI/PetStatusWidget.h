/**
* @file PetStatusWidget.h
* @brief Main HUD에 표시되는 Pet 하나의 정보를 표시
* - Pet의 아이콘, 이름 등을 표시
* - 추후 적용 효과, 체력 등도 표시할 수 있음
*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/StreamableManager.h"

#include "PetStatusWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;
class APetCompanionCharacter;
class UTexture2D;

UCLASS()
class BORNTOENDURE_API UPetStatusWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(meta = (BindWidget))
	UButton* PetSelectButton;

	UPROPERTY(meta = (BindWidget))
	UImage* PetIconImage;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PetNameText;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPetStatusSelected, APetCompanionCharacter*, SelectedPet);
	FOnPetStatusSelected OnPetStatusSelected;

	virtual void NativeOnInitialized() override;

	void InitializeWithPetData(APetCompanionCharacter* InPet);

private:
	TSharedPtr<FStreamableHandle> TextureLoadingHandle;
	TWeakObjectPtr<APetCompanionCharacter> PetCharacter;

	UFUNCTION()
	void HandlePetSelectButtonClicked();

	void SetPetIconImage(TSoftObjectPtr<UTexture2D> PetIcon);
	void OnIconsLoaded(TSoftObjectPtr<UTexture2D> LoadedIcons);
};
