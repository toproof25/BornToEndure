/**
 * @file PlayerHUDWidget.h
 * @brief 플레이어의 HUD를 관리하는 위젯 헤더
 * @author toproof (kmnlmn123@gmail.com)
 * @date 2026-05-19
 * @details
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



UCLASS()
class BORNTOENDURE_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	virtual void NativeOnInitialized() override;

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* RootCanvasPanel;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* LevelText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ExpBar;

public:

	UFUNCTION()
	void UpdateExpBar(float NewExp, float MaxExp);

	UFUNCTION()
	void UpdateLevelText(int32 NewLevel);



};
