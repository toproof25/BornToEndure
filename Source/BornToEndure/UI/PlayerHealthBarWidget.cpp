#include "UI/PlayerHealthBarWidget.h"
#include "Components/ProgressBar.h"
#include "Component/PlayerHealthComponent.h"

void UPlayerHealthBarWidget::InitializeWidget(UPlayerHealthComponent* InHealthComp)
{
	if (!InHealthComp) return;

	UnbindDelegate();

	PlayerHealthComp = InHealthComp;
	PlayerHealthComp->OnPlayerHealthUpdate.AddUObject(this, &UPlayerHealthBarWidget::UpdateHealthBar);
	UpdateHealthBar(1.f, 1.f);

	// 추후 PlayerHealthComp에서 체력 초기화 후 Delegate 호출 시 Bar 초기화할 예정
}

void UPlayerHealthBarWidget::UpdateHealthBar(float NewHealth, float MaxHealth)
{
	HealthBar->SetPercent(MaxHealth > 0.f ? NewHealth / MaxHealth : 0.f);
}

void UPlayerHealthBarWidget::NativeDestruct()
{
	UnbindDelegate();
	Super::NativeDestruct();
}

void UPlayerHealthBarWidget::UnbindDelegate()
{
	if (PlayerHealthComp)
	{
		PlayerHealthComp->OnPlayerHealthUpdate.RemoveAll(this);
	}
}
