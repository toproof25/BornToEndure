/**
 * @file LobbyPetEntryWidget.h
 * @brief 로비 Pet 카드의 아이콘, 이름, 선택 강조 및 클릭 계약을 정의합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyPetEntryWidget.generated.h"

class UBorder;
class UButton;
class UImage;
class UTextBlock;
struct FPetDataRow;

UCLASS()
class BORNTOENDURE_API ULobbyPetEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnLobbyPetEntryClicked, FName);
	FOnLobbyPetEntryClicked OnPetClicked;

	/** @param RowName RowID 필드와 별개인 실제 DataTable 행 이름입니다. */
	void InitializePet(FName RowName, const FPetDataRow& Row);
	FName GetPetRowName() const;
	void SetSelected(bool bSelected);

protected:
	virtual void NativeOnInitialized() override;

private:
	UFUNCTION()
	void HandlePetButtonClicked();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> PetButton = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PetIcon = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PetNameText = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> SelectionBorder = nullptr;

	FName PetRowName = NAME_None;
};
