/**
* @file PetDetailWidget.h
* @brief Pet의 상세 정보를 표시하는 위젯
* - 기본적으로 정보를 확인하는 Pet의 참조를 받아서 데이터를 업데이트함
*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PetDetailWidget.generated.h"

class APetCompanionCharacter;
class UPetItemWidget;
class UPetStatWidget;

UCLASS()
class BORNTOENDURE_API UPetDetailWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPetItemWidget> PetItemWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPetStatWidget> PetStatWidget;

	virtual void NativeOnInitialized() override;
	void UpdatePetDetail(APetCompanionCharacter* InPet);
};
