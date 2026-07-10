#include "Core/LobbyPlayerController.h"
#include "UI/LobbyWidget.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"

ALobbyPlayerController::ALobbyPlayerController()
{
}

void ALobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

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
	if (GameplayLevel.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("GameplayLevel is not set. Please set a valid level in the LobbyPlayerController."));
		return;
	}

	UGameplayStatics::OpenLevelBySoftObjectPtr(
		this,
		GameplayLevel,
		true,
		FString()
	);
	UE_LOG(LogTemp, Log, TEXT("Traveling to Gameplay Level: %s"), *GameplayLevel.ToString());

}
