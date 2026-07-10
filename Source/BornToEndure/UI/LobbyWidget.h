/**
* @file LobbyWidget.h
* @brief 로비 화면 위젯 정의
*/
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyWidget.generated.h"

class UImage;
class UButton;
class UTextBlock;

UCLASS()
class BORNTOENDURE_API ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	UImage* BackgroundImage;

	UPROPERTY(meta = (BindWidget))
	UButton* StartButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StartButtonText;

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnStartButtonClicked();

};
