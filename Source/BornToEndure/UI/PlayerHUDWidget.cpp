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
#include "PlayerExpBarWidget.h"

#include "PlayerState/CombatPlayerState.h"


void UPlayerHUDWidget::InitializeWidget(APlayerCharacter* PlayerCharacter)
{
	if (!PlayerCharacter) return;

	UPlayerHealthComponent* HealthComp = PlayerCharacter->GetPlayerHealthComp();
	if (HealthComp)
	{
		HealthBarWidget->InitializeWidget(HealthComp);
		GameOverWidget->InitializeWidget(HealthComp);
	}

	ACombatPlayerState* PlayerState = PlayerCharacter->GetPlayerState<ACombatPlayerState>();
	UPlayerExperienceComponent* ExperienceComp = PlayerState ? PlayerState->GetPlayerExperienceComponent() : nullptr;
	if (PlayerState && ExperienceComp)
	{
		PlayerExpBarWidget->InitializeWidget(ExperienceComp);
	}
}
