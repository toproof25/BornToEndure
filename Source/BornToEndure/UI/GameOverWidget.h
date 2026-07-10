/**
* @file GameOverWidget.h
* @brief 게임 종료 화면 위젯 클래스
*
*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameOverWidget.generated.h"

class UButton;
class UTextBlock;
class UPlayerHealthComponent;

UCLASS()
class BORNTOENDURE_API UGameOverWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ExitDescriptionText;

	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;

	void InitializeWidget(UPlayerHealthComponent* InPlayerHealthComp);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnExitButtonClicked();

private:

	UFUNCTION()
	void OnGameOver();

	UPlayerHealthComponent* PlayerHealthComp;
};
