/**
* @file PlayerExpBarWidget.h
* @brief 플레이어 경험치 바와 레벨을 표시하는 Widget Class
* 
* - PlayerExperienceComponent에 Delegate를 초기화 하여 경험치와 레벨을 표시
*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerExpBarWidget.generated.h"

class UTextBlock;
class UProgressBar;
class UPlayerExperienceComponent;

UCLASS()
class BORNTOENDURE_API UPlayerExpBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(meta = (BindWidget))
	UTextBlock* LevelText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ExpBar;

	void InitializeWidget(UPlayerExperienceComponent* InPlayerExperienceComponent);

protected:
	virtual void NativeDestruct() override;

private:
	UPlayerExperienceComponent* PlayerExperienceComp;

	UFUNCTION()
	void UpdateExpBar(float NewExp, float MaxExp);
	UFUNCTION()
	void UpdateLevelText(int32 NewLevel);
};
