/**
 * @file LobbyPlayerController.cpp
 * @brief 로비 입력과 UI를 초기화하고 스타트 Pet 선택을 확인해 본 게임으로 이동합니다.
 */
#include "Core/LobbyPlayerController.h"

#include "Core/BornToEndureGameInstance.h"
#include "Engine/Engine.h"
#include "Misc/PackageName.h"
#include "UI/LobbyWidget.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"

ALobbyPlayerController::ALobbyPlayerController()
{
}

void ALobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UBornToEndureGameInstance* GameInstance = GetGameInstance<UBornToEndureGameInstance>())
	{
		GameInstance->ResetStarterPetSelection();
	}

	SetUpLobbyInputMappingContext();
	SetUpLobbyHUDWidget();
	SetUpLobbyInputMode();
}

void ALobbyPlayerController::SetUpLobbyInputMappingContext()
{
	if (!UIMappingContext) return;

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer) return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsystem) return;

	// 입력 매핑 컨텍스트를 로컬 플레이어 서브시스템에 등록 (우선순위 0)
	Subsystem->AddMappingContext(UIMappingContext, 0);
}

void ALobbyPlayerController::SetUpLobbyInputMode()
{
	if (!LobbyWidgetInstance) return;

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(LobbyWidgetInstance->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	SetShowMouseCursor(true);

	// UI 전용이면 보통 이동/시점 입력은 막는 게 자연스러움
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);
}

void ALobbyPlayerController::SetUpLobbyHUDWidget()
{
	if (LobbyWidgetClass)
	{
		LobbyWidgetInstance = CreateWidget<ULobbyWidget>(this, LobbyWidgetClass);
		if (LobbyWidgetInstance)
		{
			LobbyWidgetInstance->AddToViewport(0);
		}
	}
}

void ALobbyPlayerController::TravelToGameplayLevel()
{
	if (GameplayLevel.IsNull() || !GetWorld() || !GEngine)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ALobbyPlayerController] 본 게임 레벨 또는 이동 컨텍스트가 설정되지 않았습니다."));
		return;
	}

	const FName MapPackageName(*FPackageName::ObjectPathToPackageName(GameplayLevel.ToString()));
	UBornToEndureGameInstance* GameInstance = GetGameInstance<UBornToEndureGameInstance>();
	if (!GameInstance || !GameInstance->QueueStarterPetTravel(MapPackageName))
	{
		return;
	}

	UGameplayStatics::OpenLevelBySoftObjectPtr(
		this,
		GameplayLevel,
		true,
		FString()
	);
	UE_LOG(LogTemp, Log, TEXT("[ALobbyPlayerController] 본 게임 레벨로 이동 요청: %s"), *MapPackageName.ToString());
}
