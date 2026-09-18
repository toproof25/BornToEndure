/**
 * @file PlayerHUDWidget.h
 * @brief 플레이어의 HUD를 관리하는 위젯 헤더
 * 
 * - 이 파일은 플레이어의 Main HUD를 관리를 목적으로 구현
 * - 경험치 Bar과 레벨 텍스트를 관리한다
 */
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/GameTypes.h"

#include "PlayerHUDWidget.generated.h"

class UCanvasPanel;
class UTextBlock;
class UProgressBar;
class UPlayerHealthBarWidget;
class APlayerCharacter;
class APetCompanionCharacter;
class UGameOverWidget;
class UPlayerExpBarWidget;
class UGameStatusWidget;
class ULevelUpRewardWidget;
class UPetSelectionWidget;
class UPetManagerComponent;
class UPetDetailWidget;

UCLASS()
class BORNTOENDURE_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> RootCanvasPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPlayerHealthBarWidget> HealthBarWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPlayerExpBarWidget> PlayerExpBarWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPetSelectionWidget> PetSelectionWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPetDetailWidget> PetDetailWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UGameOverWidget> GameOverWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UGameStatusWidget> GameStatusWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UI)
	TSubclassOf<ULevelUpRewardWidget> LevelUpWidgetClass;

	void InitializeWidget(APlayerCharacter* PlayerCharacter);

	void ShowLevelUpWidget(FLevelUpDataBundle LevelUpData);

	bool TogglePetDetailWidget();

private:

	bool bIsPetDetailWidgetVisible = false;

	UFUNCTION()
	void HandlePetAddAndRemove(UPetManagerComponent* PetManagerComp);

	UFUNCTION()
	void HandlePetSelected(APetCompanionCharacter* InSelectedPet);
};
