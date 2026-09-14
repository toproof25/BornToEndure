/**
* @file PetStatWidget.h
* @brief Pet에 따라 현재 Pet의 능력치 정보를 표시하는 위젯
* - PetDetailWidget에서 Pet의 참조를 받아서 데이터를 업데이트함
* - 해당 코드는 능력치가 많고 반복적이라 AI로 작성함
*/
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Stat/PetStatTypes.h"

#include "PetStatWidget.generated.h"

class UTextBlock;
class UGridPanel;

UCLASS()
class BORNTOENDURE_API UPetStatWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pet Stats")
	int32 FontSize = 14;

	/**
	 * @brief 각 스탯 텍스트를 업데이트하는 함수
	 * @param PetStats Pet의 최종 스탯 정보
	 */
	void UpdatePetStats(const TMap<EPetStatType, float>& PetStats);

protected:
	virtual void NativeOnInitialized() override;


private:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UGridPanel> StatGridPanel;

	UPROPERTY(Transient)
	TMap<EPetStatType, TObjectPtr<UTextBlock>> StatValueTexts;

	void InitializeStatGrid();
	void AddStatRow(EPetStatType StatType, const FText& LabelText, int32 RowIndex);

};
