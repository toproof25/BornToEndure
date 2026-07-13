/**
* @file LobbyPlayerController.h
* @brief Lobby 화면을 제어하는 PlayerController
* 
* - Lobby Widget을 띄우며 상호작용
* - Lobby 화면에 필요한 기능들을 제공
*/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyPlayerController.generated.h"

class ULobbyWidget;
class UInputMappingContext;
class UUserWidget;

UCLASS()
class BORNTOENDURE_API ALobbyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ALobbyPlayerController();

	void TravelToGameplayLevel();

protected:
	virtual void BeginPlay() override;

private:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> UIMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true", AllowedClasses = "LobbyWidget"))
	TSubclassOf<ULobbyWidget> LobbyWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Lobby|Travel", meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UWorld> GameplayLevel;

	UPROPERTY()
	TObjectPtr<ULobbyWidget> LobbyWidgetInstance;

	//void SetUpDelegates();
	void SetUpLobbyInputMappingContext();
	void SetUpLobbyInputMode();
	void SetUpLobbyHUDWidget();

};
