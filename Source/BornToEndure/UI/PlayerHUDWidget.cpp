#include "UI/PlayerHUDWidget.h"

#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/CanvasPanel.h"

#include "UI/PlayerHealthBarWidget.h"
#include "UI/GameOverWidget.h"
#include "UI/GameStatusWidget.h"
#include "UI/LevelUpRewardWidget.h"
#include "UI/PlayerExpBarWidget.h"
#include "UI/PetSelectionWidget.h"
#include "UI/PetDetailWidget.h"

#include "PlayerState/CombatPlayerState.h"
#include "Character/Player/PlayerCharacter.h"
#include "Component/PlayerExperienceComponent.h"
#include "Component/PlayerHealthComponent.h"
#include "Component/PetManagerComponent.h"
#include "Character/Pet/PetCompanionCharacter.h"

#include "Data/GameTypes.h"

#include "PlayerState/CombatPlayerState.h"


void UPlayerHUDWidget::InitializeWidget(APlayerCharacter* PlayerCharacter)
{
	if (!PlayerCharacter) return;

	UPlayerHealthComponent* HealthComp = PlayerCharacter->GetPlayerHealthComp();
	ACombatPlayerState* PlayerState = PlayerCharacter->GetPlayerState<ACombatPlayerState>();
	UPlayerExperienceComponent* ExperienceComp = PlayerState ? PlayerState->GetPlayerExperienceComponent() : nullptr;
	UPetManagerComponent* PetManagerComp = PlayerCharacter->GetPetManagerComp();

	if (HealthComp)
	{
		HealthBarWidget->InitializeWidget(HealthComp);
		GameOverWidget->InitializeWidget(HealthComp);
		UE_LOG(LogTemp, Warning, TEXT("[UPlayerHUDWidget] HealthComp 초기화 완료"));
	}

	if (PlayerState && ExperienceComp)
	{
		PlayerExpBarWidget->InitializeWidget(ExperienceComp);
		UE_LOG(LogTemp, Warning, TEXT("[UPlayerHUDWidget] PlayerState와 ExperienceComp 초기화 완료"));
	}
	
	if (HealthComp && PlayerState)
	{
		GameStatusWidget->InitializeWidget(HealthComp, PlayerState);
		UE_LOG(LogTemp, Warning, TEXT("[UPlayerHUDWidget] GameStatusWidget 초기화 완료"));
	}

	if (PetManagerComp)
	{
		PetSelectionWidget->OnPetSelected.AddDynamic(this, &UPlayerHUDWidget::HandlePetSelected);

		PetManagerComp->OnPetAdded.AddUObject(this, &UPlayerHUDWidget::HandlePetAddAndRemove);
		PetManagerComp->OnPetRemoved.AddUObject(this, &UPlayerHUDWidget::HandlePetAddAndRemove);
		HandlePetAddAndRemove(PetManagerComp);
		UE_LOG(LogTemp, Warning, TEXT("[UPlayerHUDWidget] PetManagerComp 초기화 완료"));
	}

	if (PetDetailWidget && PetManagerComp)
	{
		PetManagerComp->OnPetItemReceived.AddUObject(PetDetailWidget, &UPetDetailWidget::UpdatePetDetail);
		PetDetailWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPlayerHUDWidget::ShowLevelUpWidget(FLevelUpDataBundle LevelUpData)
{
	ULevelUpRewardWidget* LevelUpWidget = CreateWidget<ULevelUpRewardWidget>(GetWorld(), LevelUpWidgetClass);

	if (LevelUpWidget)
	{
		LevelUpWidget->InitializeWithLevelUpData(LevelUpData); /// 레벨업 보상 창에 데이터 전달
		LevelUpWidget->AddToViewport(100);
		UE_LOG(LogTemp, Warning, TEXT("[ADefaultPlayerController] 레벨업 창 활성화"));
	}
}

bool UPlayerHUDWidget::TogglePetDetailWidget()
{
	if (!PetDetailWidget) return false;

	bIsPetDetailWidgetVisible = !bIsPetDetailWidgetVisible;
	PetDetailWidget->SetVisibility(bIsPetDetailWidgetVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	UE_LOG(LogTemp, Warning, TEXT("[UPlayerHUDWidget] TogglePetDetailWidget: %s"), bIsPetDetailWidgetVisible ? TEXT("Visible") : TEXT("Hidden"));
	return bIsPetDetailWidgetVisible;
}

void UPlayerHUDWidget::HandlePetAddAndRemove(UPetManagerComponent* PetManagerComp)
{
	PetSelectionWidget->InitializeWidget(PetManagerComp);
	UE_LOG(LogTemp, Warning, TEXT("[UPlayerHUDWidget] PetManagerComp 초기화 완료"));
}

void UPlayerHUDWidget::HandlePetSelected(APetCompanionCharacter* InSelectedPet)
{
	UE_LOG(LogTemp, Warning, TEXT("[UPlayerHUDWidget] HandlePetSelected: 선택된 펫: %s"), *InSelectedPet->GetName());

	// 능력치 위젯, 아이템 위젯에 데이터를 전달
	PetDetailWidget->UpdatePetDetail(InSelectedPet);
}
