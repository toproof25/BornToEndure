/**
* @file PetSelectionWidget.h
* @brief 플레이시 화면에 보이는 펫 목록과 펫 선택 UI를 관리하는 위젯 헤더
*/
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/StreamableManager.h"
#include "PetSelectionWidget.generated.h"

class UVerticalBox;
class UImage;
class UButton;

class UPetManagerComponent;
class APetCompanionCharacter;
class UPetStatusWidget;



UCLASS()
class BORNTOENDURE_API UPetSelectionWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> PetVerticalBox;
	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UI)
	TSubclassOf<UPetStatusWidget> PetStatusWidgetClass;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPetSelected, APetCompanionCharacter*, SelectedPet);
	FOnPetSelected OnPetSelected;

	virtual void NativeOnInitialized() override;

	void InitializeWidget(const UPetManagerComponent* PetManagerComponent);

protected:

private:
	TWeakObjectPtr<APetCompanionCharacter> SelectedPet;

	UFUNCTION()
	void HandlePetStatusSelected(APetCompanionCharacter* InSelectedPet);
};
