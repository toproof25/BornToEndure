/** 
* @file GameStatusWidget.h
* @brief 게임 전투 통계를 표시하는 위젯
* 
* - PlayerState와 연동하여 게임 전투 통계를 표시하는 위젯
*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "GameStatusWidget.generated.h"

class ACombatPlayerState;
class UGameStatusRowWidget;
class UVerticalBox;
class UPlayerHealthComponent;

UCLASS()
class BORNTOENDURE_API UGameStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* GameStatusList;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameStatus")
	TSubclassOf<UGameStatusRowWidget> GameStatusRowWidgetClass;

	void InitializeWidget(UPlayerHealthComponent* InPlayerHealthComponent, ACombatPlayerState* InCombatPlayerState);
	void UpdateGameStatus();

protected:
	virtual void NativeDestruct() override;

private:

	void ShowGameStatus();

	UPlayerHealthComponent* PlayerHealthComp;
	ACombatPlayerState* CombatPlayerState;
};
