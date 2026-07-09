/**
* @file GameStatusRowWidget.h
* @brief 게임 통계 위젯의 행을 나타내는 위젯
* 
* - Pet의 아이콘, 이름, 잡은 적 수, 총 데미지 등을 나열
*/
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameStatusRowWidget.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;


UCLASS()
class BORNTOENDURE_API UGameStatusRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	UImage* Icon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Name;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TotalCount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TotalDamage;

	void InitializeWidget(UTexture2D* InIcon, FName InName, int32 InTotalCount, int32 InTotalDamage);


};
