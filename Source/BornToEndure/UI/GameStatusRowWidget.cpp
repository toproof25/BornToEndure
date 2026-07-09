#include "UI/GameStatusRowWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

void UGameStatusRowWidget::InitializeWidget(UTexture2D* InIcon, FName InName, int32 InTotalCount, int32 InTotalDamage)
{
	if (Icon)
	{
		Icon->SetBrushFromTexture(InIcon);
	}
	
	if (Name)
	{
		Name->SetText(FText::FromName(InName));
	}

	if (TotalCount)
	{
		TotalCount->SetText(FText::AsNumber(InTotalCount));
	}

	if (TotalDamage)
	{
		TotalDamage->SetText(FText::AsNumber(InTotalDamage));
	}
	
	
	
}
