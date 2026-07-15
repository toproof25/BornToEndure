#include "Core/DefaultPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"

#include "UI/LevelUpRewardWidget.h"
#include "UI/PlayerHUDWidget.h"
#include "UI/PlayerHealthBarWidget.h"

#include "Data/GameTypes.h"
#include "Data/DataTableRow/ItemDataRow.h"
#include "Subsystem/ItemPoolSubsystem.h"
#include "PlayerState/CombatPlayerState.h"

#include "Component/PlayerExperienceComponent.h"
#include "Component/PlayerHealthComponent.h"
#include "Component/PetManagerComponent.h"

#include "Character/Pet/PetCompanionCharacter.h"
#include "Character/Player/PlayerCharacter.h"


void ADefaultPlayerController::SetGameInputMode()
{
	FInputModeGameOnly InputMode;
	InputMode.SetConsumeCaptureMouseDown(false);

	SetInputMode(InputMode);

	SetShowMouseCursor(false);

	// UI에서 막았던 이동/시점 입력 복구
	ResetIgnoreInputFlags();

	// 게임 일시정지도 같이 관리할 거면 여기서 해제
	SetPause(false);
}

void ADefaultPlayerController::SetUIInputMode(UUserWidget* WidgetToFocus, bool bInPauseGame)
{
	if (!WidgetToFocus) return;
	
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(WidgetToFocus->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	SetInputMode(InputMode);

	SetShowMouseCursor(true);

	// UI 전용이면 보통 이동/시점 입력은 막는 게 자연스러움
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);

	SetPause(bInPauseGame);
}

void ADefaultPlayerController::SetGameAndUIInputMode(UUserWidget* WidgetToFocus, bool bIgnoreMoveInput, bool bIgnoreLookInput)
{
	if (!WidgetToFocus) return;

	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(WidgetToFocus->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);

	SetInputMode(InputMode);

	SetShowMouseCursor(true);

	if (bIgnoreMoveInput)
	{
		SetIgnoreMoveInput(true);
	}

	if (bIgnoreLookInput)
	{
		SetIgnoreLookInput(true);
	}
}

void ADefaultPlayerController::RestoreGameInputMode()
{
	SetGameInputMode();
}

void ADefaultPlayerController::TravelToLobbyLevel()
{
	if (LobbyLevel.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("LobbyLevel is not set. Please set a valid level in the DefaultPlayerController."));
		return;
	}

	UGameplayStatics::OpenLevelBySoftObjectPtr(
		this,
		LobbyLevel,
		true,
		FString()
	);
	UE_LOG(LogTemp, Log, TEXT("Traveling to Lobby Level: %s"), *LobbyLevel.ToString());
}

void ADefaultPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetUpPlayerInputMode();
	SetUpPlayerHUDWidget();
	SetUpDelegates();
	SetGameInputMode();
}

void ADefaultPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	SetUpDelegates();
}

void ADefaultPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (PlayerExpComp && PlayerHUDWidgetInstance)
	{
		PlayerExpComp->OnChangeExpDelegate.RemoveAll(PlayerHUDWidgetInstance);
		PlayerExpComp->OnLevelUpDelegate.RemoveAll(PlayerHUDWidgetInstance);
		PlayerExpComp->OnLevelUpDelegate.RemoveAll(this);
	}
	Super::EndPlay(EndPlayReason);
}

void ADefaultPlayerController::SetUpDelegates()
{
	if (!PlayerHUDWidgetInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ADefaultPlayerController] PlayerHUDWidgetInstance is null. Cannot set up delegates."));
		return;
	}

	ACombatPlayerState* PS = GetPlayerState<ACombatPlayerState>();
	if (!PS)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ADefaultPlayerController] PlayerState is not of type ACombatPlayerState or is null."));
		return;
	}

	PlayerExpComp = PS->GetPlayerExperienceComponent();
	if (!PlayerExpComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ADefaultPlayerController] PlayerExperienceComponent not found in PlayerState."));
		return;
	}

	APlayerCharacter* PlayerCharacter = GetPawn<APlayerCharacter>();
	if (!PlayerCharacter) return;
	UPlayerHealthComponent* HealthComp = PlayerCharacter->GetPlayerHealthComp();
	if (!HealthComp) return;

	PlayerHUDWidgetInstance->InitializeWidget(PlayerCharacter); // UI Delegate 초기화

	HealthComp->OnPlayerDeath.RemoveAll(this);
	PlayerExpComp->OnLevelUpDelegate.RemoveAll(this);

	HealthComp->OnPlayerDeath.AddUObject(this, &ADefaultPlayerController::HandlePlayerDeath); /// 플레이어 사망 시 처리
	PlayerExpComp->OnLevelUpDelegate.AddDynamic(this, &ADefaultPlayerController::LevelUpHandler); /// 레벨업 시 레벨업 보상 창 Widget 활성화

	UE_LOG(LogTemp, Warning, TEXT("[ADefaultPlayerController] DefaultPlayerController Delegate SetUp"));
}

void ADefaultPlayerController::SetUpPlayerInputMode()
{
	// 로컬 플레이어 가져오기 (멀티플레이 시, 다른 유저 컨트롤러에서는 이 값이 null이 된다)
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer) return;

	// 로컬 플레이어에 입력 서브시스템을 가져옵니다.
	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsystem) return;

	// 입력 매핑 컨텍스트를 로컬 플레이어 서브시스템에 등록 (우선순위 0)
	Subsystem->AddMappingContext(DefaultMappingContext, 0);
	UE_LOG(LogTemp, Warning, TEXT("[ADefaultPlayerController] PlayerCharacter BeginPlay called. DefaultMappingContext"));
}

void ADefaultPlayerController::SetUpPlayerHUDWidget()
{
	// Player 메인 UI 적용
	if (PlayerHUDWidgetClass)
	{
		PlayerHUDWidgetInstance = CreateWidget<UPlayerHUDWidget>(this, PlayerHUDWidgetClass);
		if (PlayerHUDWidgetInstance)
		{
			PlayerHUDWidgetInstance->AddToViewport(0);
		}
	}
}

void ADefaultPlayerController::HandlePlayerDeath()
{
	UE_LOG(LogTemp, Warning, TEXT("[ADefaultPlayerController] Player has died. Handling death."));
	// 플레이어가 죽었을 때 처리할 로직을 여기에 추가
	// 예: 게임 오버 화면 표시, 리스폰 처리 등
}

void ADefaultPlayerController::LevelUpHandler(int32 NewLevel)
{
	// Pet 목록 가져오기
	FLevelUpDataBundle LevelUpData;
	UPetManagerComponent* PetManager = GetPawn()->FindComponentByClass<UPetManagerComponent>();
	LevelUpData.PetList = PetManager ? PetManager->GetPetList() : TArray<TObjectPtr<APetCompanionCharacter>>(); 

	// 랜덤 Item 가져오기
	UWorld* World = GetWorld();
	if (!World) return;
	UItemPoolSubsystem* ItemPool = World->GetGameInstance()->GetSubsystem<UItemPoolSubsystem>(); /// GameInstance에서 ItemPoolSubsystem을 가져옴
	if (!ItemPool) return;
	TArray<TObjectPtr<UObject>> RandomItems = ItemPool->GetRandomItemObjects(3); /// 레벨업 보상으로 3개의 랜덤 아이템을 가져옴
	LevelUpData.RandomItemList = RandomItems;

	UE_LOG(LogTemp, Warning, TEXT("[ADefaultPlayerController] LevelUpHandler: 가져온 랜덤 아이템 개수 - %d"), RandomItems.Num());

	PlayerHUDWidgetInstance->ShowLevelUpWidget(LevelUpData); /// PlayerHUDWidget에 레벨업 보상 창 활성화 요청
}
