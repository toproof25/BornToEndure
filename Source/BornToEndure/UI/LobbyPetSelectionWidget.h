/**
 * @file LobbyPetSelectionWidget.h
 * @brief 로비에서 스타트 Pet 목록을 보여 주고 한 마리의 선택 상태를 표시합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyPetSelectionWidget.generated.h"

class UHorizontalBox;
class ULobbyPetEntryWidget;

UCLASS()
class BORNTOENDURE_API ULobbyPetSelectionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnLobbyPetSelectionChanged, FName);
	FOnLobbyPetSelectionChanged OnPetSelectionChanged;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void BuildPetList();
	void HandlePetEntryClicked(FName RowName);
	void RefreshSelection(FName RowName);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> PetListBox = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Lobby|Pet")
	TSubclassOf<ULobbyPetEntryWidget> PetEntryWidgetClass = nullptr;
};
