/** 
* @file PlayerHealthBarWidget.h
* @brief 플레이어의 체력바를 관리하는 위젯 헤더
* 
* 
*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHealthBarWidget.generated.h"

class UProgressBar;
class UPlayerHealthComponent;

UCLASS()
class BORNTOENDURE_API UPlayerHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta=(BindWidget))
	UProgressBar* HealthBar;

	void InitializeWidget(UPlayerHealthComponent* InHealthComp);
	void UpdateHealthBar(float NewHealth, float MaxHealth);

protected:
	virtual void NativeDestruct() override;


private:
	TObjectPtr<UPlayerHealthComponent> PlayerHealthComp;

	void UnbindDelegate();

};
