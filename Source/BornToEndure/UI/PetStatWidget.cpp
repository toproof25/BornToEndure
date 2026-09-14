#include "UI/PetStatWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/GridPanel.h"
#include "Components/GridSlot.h"

#include "Stat/PetStatTypes.h"

void UPetStatWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	InitializeStatGrid();
}

void UPetStatWidget::UpdatePetStats(const TMap<EPetStatType, float>& PetStats)
{
	for (const TPair<EPetStatType, float>& Stat : PetStats)
	{
		TObjectPtr<UTextBlock>* ValueText = StatValueTexts.Find(Stat.Key);

		if (!ValueText || !IsValid(ValueText->Get()))
		{
			continue;
		}

		const float Value = Stat.Value;

		switch (Stat.Key)
		{
		case EPetStatType::CriticalRate:
		case EPetStatType::FireDamageBonus:
		case EPetStatType::IceDamageBonus:
		case EPetStatType::WindDamageBonus:
		case EPetStatType::PoisonDamageBonus:
		case EPetStatType::BleedDamageBonus:
			(*ValueText)->SetText(FText::AsPercent(Value));
			break;
		case EPetStatType::CriticalDamage:
		{
			(*ValueText)->SetText(
				FText::Format(
					NSLOCTEXT(
						"PetStatsWidget",
						"MultiplierFormat",
						"{0}x"),
					FText::AsNumber(Value)));

			break;
		}
		default:
			(*ValueText)->SetText(FText::AsNumber(Value));
			break;
		}
	}
}


#define LOCTEXT_NAMESPACE "PetStatsWidget"

void UPetStatWidget::InitializeStatGrid()
{
	if (!StatGridPanel)
	{
		return;
	}

	StatGridPanel->ClearChildren();
	StatValueTexts.Empty();

	int32 Row = 0;

	AddStatRow(
		EPetStatType::AttackPower,
		LOCTEXT("AttackPower", "공격력"),
		Row++);

	AddStatRow(
		EPetStatType::MaxHealth,
		LOCTEXT("MaxHealth", "최대 체력"),
		Row++);

	AddStatRow(
		EPetStatType::CurrentHealth,
		LOCTEXT("CurrentHealth", "현재 체력"),
		Row++);

	AddStatRow(
		EPetStatType::Defense,
		LOCTEXT("Defense", "방어력"),
		Row++);

	AddStatRow(
		EPetStatType::CriticalRate,
		LOCTEXT("CriticalRate", "치명타 확률"),
		Row++);

	AddStatRow(
		EPetStatType::CriticalDamage,
		LOCTEXT("CriticalDamage", "치명타 데미지"),
		Row++);

	AddStatRow(
		EPetStatType::AttackSpeed,
		LOCTEXT("AttackSpeed", "공격 속도"),
		Row++);

	AddStatRow(
		EPetStatType::MoveSpeed,
		LOCTEXT("MoveSpeed", "이동 속도"),
		Row++);

	AddStatRow(
		EPetStatType::DetectionRange,
		LOCTEXT("DetectionRange", "적 탐지 범위"),
		Row++);

	AddStatRow(
		EPetStatType::AttackRange,
		LOCTEXT("AttackRange", "공격 범위"),
		Row++);

	AddStatRow(
		EPetStatType::Luck,
		LOCTEXT("Luck", "행운"),
		Row++);

	AddStatRow(
		EPetStatType::FireDamageBonus,
		LOCTEXT("FireDamageBonus", "화염 데미지 보너스"),
		Row++);

	AddStatRow(
		EPetStatType::IceDamageBonus,
		LOCTEXT("IceDamageBonus", "냉기 데미지 보너스"),
		Row++);

	AddStatRow(
		EPetStatType::WindDamageBonus,
		LOCTEXT("WindDamageBonus", "바람 데미지 보너스"),
		Row++);

	AddStatRow(
		EPetStatType::PoisonDamageBonus,
		LOCTEXT("PoisonDamageBonus", "독 데미지 보너스"),
		Row++);

	AddStatRow(
		EPetStatType::BleedDamageBonus,
		LOCTEXT("BleedDamageBonus", "출혈 데미지 보너스"),
		Row++);
}

#undef LOCTEXT_NAMESPACE

void UPetStatWidget::AddStatRow(EPetStatType StatType, const FText& LabelText, int32 RowIndex)
{
	if (!StatGridPanel || !WidgetTree)
	{
		return;
	}

	UTextBlock* LabelTextBlock =
		WidgetTree->ConstructWidget<UTextBlock>();

	UTextBlock* ValueTextBlock =
		WidgetTree->ConstructWidget<UTextBlock>();

	FSlateFontInfo LabelFont = LabelTextBlock->GetFont();
	LabelFont.Size = FontSize;
	LabelTextBlock->SetFont(LabelFont);

	FSlateFontInfo ValueFont = ValueTextBlock->GetFont();
	ValueFont.Size = FontSize;
	ValueTextBlock->SetFont(ValueFont);

	if (!LabelTextBlock || !ValueTextBlock)
	{
		return;
	}

	LabelTextBlock->SetText(LabelText);
	ValueTextBlock->SetText(FText::AsNumber(0));

	// 왼쪽 Label
	UGridSlot* LabelSlot =
		StatGridPanel->AddChildToGrid(
			LabelTextBlock,
			RowIndex,
			0);

	if (LabelSlot)
	{
		LabelSlot->SetHorizontalAlignment(HAlign_Left);
		LabelSlot->SetVerticalAlignment(VAlign_Center);
		LabelSlot->SetPadding(
			FMargin(4.0f, 3.0f, 20.0f, 3.0f));
	}

	// 오른쪽 Value
	UGridSlot* ValueSlot =
		StatGridPanel->AddChildToGrid(
			ValueTextBlock,
			RowIndex,
			1);

	if (ValueSlot)
	{
		ValueSlot->SetHorizontalAlignment(HAlign_Right);
		ValueSlot->SetVerticalAlignment(VAlign_Center);
		ValueSlot->SetPadding(
			FMargin(4.0f, 3.0f, 4.0f, 3.0f));
	}

	StatValueTexts.Add(
		StatType,
		ValueTextBlock);
}
