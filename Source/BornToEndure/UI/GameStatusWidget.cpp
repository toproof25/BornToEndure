#include "UI/GameStatusWidget.h"

#include "PlayerState/CombatPlayerState.h"
#include "Components/VerticalBox.h"
#include "UI/GameStatusRowWidget.h"
#include "Component/PlayerHealthComponent.h"
#include "Data/GameTypes.h"

void UGameStatusWidget::InitializeWidget(UPlayerHealthComponent* InPlayerHealthComponent, ACombatPlayerState* InCombatPlayerState)
{
	if (!InPlayerHealthComponent || !InCombatPlayerState) return;

	PlayerHealthComp = InPlayerHealthComponent;
	CombatPlayerState = InCombatPlayerState;

	GameStatusList->ClearChildren();

	PlayerHealthComp->OnPlayerDeath.AddUObject(this, &UGameStatusWidget::UpdateGameStatus);

	this->SetVisibility(ESlateVisibility::Hidden);
}

void UGameStatusWidget::UpdateGameStatus()
{
	if (CombatPlayerState)
	{
		this->SetVisibility(ESlateVisibility::Visible);
		GameStatusList->ClearChildren();

		const TMap<FName, float>& PetDamageStats = CombatPlayerState->GetPetDamageStats();
		for (auto RowData : PetDamageStats)
		{
			UGameStatusRowWidget* NewRowWidget = CreateWidget<UGameStatusRowWidget>(this, GameStatusRowWidgetClass);
			NewRowWidget->InitializeWidget(nullptr, RowData.Key, 0, RowData.Value);
			GameStatusList->AddChildToVerticalBox(NewRowWidget);

		}
	}
}

void UGameStatusWidget::NativeDestruct()
{
	if (PlayerHealthComp)
	{
		PlayerHealthComp->OnPlayerDeath.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UGameStatusWidget::ShowGameStatus()
{

}
