/**
* @file DefaultPlayerController.h
* @brief 기본 플레이어 컨트롤러 클래스 헤더
* 
* - 플레이어 입력 처리 및 UI 관리
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "DefaultPlayerController.generated.h"

class UInputMappingContext;
class UPlayerHUDWidget;
class ULevelUpRewardWidget;
class UPlayerExperienceComponent;
class UPlayerHealthBarWidget;
class UUserWidget;

UCLASS()
class BORNTOENDURE_API ADefaultPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	void SetGameInputMode();
	void SetUIInputMode(UUserWidget* WidgetToFocus, bool bInPauseGame = false);
	void SetGameAndUIInputMode(
		UUserWidget* WidgetToFocus,
		bool bIgnoreMoveInput = true,
		bool bIgnoreLookInput = false
	);
	void RestoreGameInputMode();

	void TravelToLobbyLevel();

protected:

	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** * @brief Input Mapping Context를 편집기에서 할당 가능하도록 선언 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UI, meta = (AllowedClasses = "PlayerHUDWidget"))
	TSubclassOf<UPlayerHUDWidget> PlayerHUDWidgetClass;

private:

	UPROPERTY()
	TObjectPtr<UPlayerHUDWidget> PlayerHUDWidgetInstance;

	UPROPERTY()
	TObjectPtr<UPlayerExperienceComponent> PlayerExpComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Level", meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UWorld> LobbyLevel;

	void SetUpDelegates();
	void SetUpPlayerInputMode();
	void SetUpPlayerHUDWidget();

	void HandlePlayerDeath();

	UFUNCTION()
	void LevelUpHandler(int32 NewLevel);



};
