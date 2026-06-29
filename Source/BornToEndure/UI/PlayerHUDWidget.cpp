// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerHUDWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/CanvasPanel.h"

#include "PlayerState/CombatPlayerState.h"
#include "Component/PlayerExperienceComponent.h"

void UPlayerHUDWidget::NativeOnInitialized()
{
	//APawn* Player = GetOwningPlayer()->GetPawn();
	//if (!Player) return;
	//ACombatPlayerState* CombatPlayerState = Player->GetPlayerState<ACombatPlayerState>();
	//if (!CombatPlayerState) return;
	//UPlayerExperienceComponent* PlayerExpComp = CombatPlayerState->FindComponentByClass<UPlayerExperienceComponent>();
	//if (!PlayerExpComp) return;

	//PlayerExpComp->OnChangeExpDelegate.AddDynamic(this, &UPlayerHUDWidget::UpdateExpBar);
	//PlayerExpComp->OnLevelUpDelegate.AddDynamic(this, &UPlayerHUDWidget::UpdateLevelText);
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
