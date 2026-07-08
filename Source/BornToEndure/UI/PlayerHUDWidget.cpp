#include "UI/PlayerHUDWidget.h"

#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/CanvasPanel.h"

#include "UI/PlayerHealthBarWidget.h"
#include "UI/GameOverWidget.h"

#include "PlayerState/CombatPlayerState.h"
#include "Character/Player/PlayerCharacter.h"

#include "Component/PlayerExperienceComponent.h"
#include "Component/PlayerHealthComponent.h"


void UPlayerHUDWidget::InitializeWidget(APlayerCharacter* PlayerCharacter)
{
	if (!PlayerCharacter) return;

	UPlayerHealthComponent* HealthComp = PlayerCharacter->GetPlayerHealthComp();
	if (!HealthComp) return;

	HealthBarWidget->InitializeWidget(HealthComp);
	GameOverWidget->InitializeWidget(HealthComp);
}

void UPlayerHUDWidget::UpdateExpBar(float NewExp, float MaxExp)
{
	float ExpPercent = MaxExp > 0.f ? NewExp / MaxExp : 0.f;
	if (ExpBar)
	{
		ExpBar->SetPercent(ExpPercent);
	}
}

void UPlayerHUDWidget::UpdateLevelText(int32 NewLevel)
{
	if (LevelText)
	{
		LevelText->SetText(FText::Format(NSLOCTEXT("PlayerHUD", "LevelFormat", "Level {0}"), FText::AsNumber(NewLevel)));
	}
}

void UPlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

