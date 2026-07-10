#include "UI/PlayerExpBarWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Component/PlayerExperienceComponent.h"

void UPlayerExpBarWidget::InitializeWidget(UPlayerExperienceComponent* InPlayerExperienceComponent)
{
	if (!InPlayerExperienceComponent) return;

	PlayerExperienceComp = InPlayerExperienceComponent;
	PlayerExperienceComp->OnChangeExpDelegate.AddDynamic(this, &UPlayerExpBarWidget::UpdateExpBar);
	PlayerExperienceComp->OnLevelUpDelegate.AddDynamic(this, &UPlayerExpBarWidget::UpdateLevelText);
}

void UPlayerExpBarWidget::NativeDestruct()
{
	if (PlayerExperienceComp)
	{
		PlayerExperienceComp->OnChangeExpDelegate.RemoveAll(this);
		PlayerExperienceComp->OnLevelUpDelegate.RemoveAll(this);
	}

	Super::NativeDestruct();
}


void UPlayerExpBarWidget::UpdateExpBar(float NewExp, float MaxExp)
{
	if (ExpBar)
	{
		float ExpPercent = MaxExp > 0.f ? NewExp / MaxExp : 0.f;
		ExpBar->SetPercent(ExpPercent);
	}

}
void UPlayerExpBarWidget::UpdateLevelText(int32 NewLevel)
{
	if (LevelText)
	{
		LevelText->SetText(FText::Format(NSLOCTEXT("PlayerExpBar", "LevelFormat", "Level {0}"), FText::AsNumber(NewLevel)));
	}
}
