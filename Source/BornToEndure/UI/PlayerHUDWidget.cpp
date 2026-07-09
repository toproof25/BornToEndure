#include "UI/PlayerHUDWidget.h"

#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/CanvasPanel.h"

#include "UI/PlayerHealthBarWidget.h"
#include "UI/GameOverWidget.h"
#include "UI/GameStatusWidget.h"
#include "UI/LevelUpRewardWidget.h"
#include "UI/PlayerExpBarWidget.h"

#include "PlayerState/CombatPlayerState.h"
#include "Character/Player/PlayerCharacter.h"
#include "Component/PlayerExperienceComponent.h"
#include "Component/PlayerHealthComponent.h"

#include "Data/GameTypes.h"

#include "PlayerState/CombatPlayerState.h"


void UPlayerHUDWidget::InitializeWidget(APlayerCharacter* PlayerCharacter)
{
	if (!PlayerCharacter) return;

	UPlayerHealthComponent* HealthComp = PlayerCharacter->GetPlayerHealthComp();
	ACombatPlayerState* PlayerState = PlayerCharacter->GetPlayerState<ACombatPlayerState>();
	UPlayerExperienceComponent* ExperienceComp = PlayerState ? PlayerState->GetPlayerExperienceComponent() : nullptr;

	if (HealthComp)
	{
		HealthBarWidget->InitializeWidget(HealthComp);
		GameOverWidget->InitializeWidget(HealthComp);
	}

	if (PlayerState && ExperienceComp)
	{
		PlayerExpBarWidget->InitializeWidget(ExperienceComp);
	}
	
	if (HealthComp && PlayerState)
	{
		GameStatusWidget->InitializeWidget(HealthComp, PlayerState);
	}


}

void UPlayerHUDWidget::ShowLevelUpWidget(FLevelUpDataBundle LevelUpData)
{
	ULevelUpRewardWidget* LevelUpWidget = CreateWidget<ULevelUpRewardWidget>(GetWorld(), LevelUpWidgetClass);

	if (LevelUpWidget)
	{
		LevelUpWidget->InitializeWithLevelUpData(LevelUpData); /// 레벨업 보상 창에 데이터 전달
		LevelUpWidget->AddToViewport(1);
		UE_LOG(LogTemp, Warning, TEXT("[ADefaultPlayerController] 레벨업 창 활성화"));
	}
}
