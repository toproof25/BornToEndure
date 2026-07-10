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

UCLASS()
class BORNTOENDURE_API ALobbyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ALobbyPlayerController();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> UIMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true", AllowedClasses = "LobbyWidget"))
	TSubclassOf<ULobbyWidget> LobbyWidgetClass;

private:

	UPROPERTY()
	TObjectPtr<ULobbyWidget> LobbyWidgetInstance;


	//void SetUpDelegates();
	void SetUpLobbyInputMappingContext();
	void SetUpLobbyInputMode();
	void SetUpLobbyHUDWidget();


};
