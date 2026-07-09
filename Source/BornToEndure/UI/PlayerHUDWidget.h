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
#include "PlayerHUDWidget.generated.h"

class UCanvasPanel;
class UTextBlock;
class UProgressBar;
class UPlayerHealthBarWidget;
class APlayerCharacter;
class UGameOverWidget;
class UPlayerExpBarWidget;
class UGameStatusWidget;

UCLASS()
class BORNTOENDURE_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* RootCanvasPanel;

	UPROPERTY(meta = (BindWidget))
	UPlayerHealthBarWidget* HealthBarWidget;

	UPROPERTY(meta = (BindWidget))
	UPlayerExpBarWidget* PlayerExpBarWidget;

	UPROPERTY(meta = (BindWidget))
	UGameOverWidget* GameOverWidget;

	UPROPERTY(meta = (BindWidget))
	UGameStatusWidget* GameStatusWidget;

	void InitializeWidget(APlayerCharacter* PlayerCharacter);

};
