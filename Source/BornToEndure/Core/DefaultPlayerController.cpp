#include "Core/DefaultPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "UI/LevelUpRewardWidget.h"
#include "UI/PlayerHUDWidget.h"
#include "UI/PlayerHealthBarWidget.h"
#include "Component/PlayerExperienceComponent.h"
#include "Character/Player/PlayerCharacter.h"
#include "Component/PlayerHealthComponent.h"
#include "PlayerState/CombatPlayerState.h"
#include "Data/GameTypes.h"
#include "Component/PetManagerComponent.h"
#include "Character/Pet/PetCompanionCharacter.h"
#include "Subsystem/ItemPoolSubsystem.h"
#include "Data/DataTableRow/ItemDataRow.h"


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

void ADefaultPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetUpPlayerInputMode();
	SetUpPlayerHUDWidget();
	SetUpDelegates();
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
	if (LevelUpWidgetClass)
	{
		// Pet 목록 가져오기
		FLevelUpDataBundle LevelUpData;
		UPetManagerComponent* PetManager = GetPawn()->FindComponentByClass<UPetManagerComponent>();
		LevelUpData.PetList = PetManager ? PetManager->GetPetList() : TArray<TObjectPtr<APetCompanionCharacter>>(); /// 시작 직후에는 목록이 비어있음 (어차피 실제 게임 레벨업 시점에서는 있을 수 밖에 없음)

		if (LevelUpData.PetList.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("[ADefaultPlayerController] LevelUpHandler: Pet 목록이 비어 있습니다."));
			return;
		}
		else
		{
			for (TObjectPtr<APetCompanionCharacter> Pet : LevelUpData.PetList)
			{
				FName PetName = Pet ? Pet->GetPetName() : NAME_None;
				UE_LOG(LogTemp, Warning, TEXT("[ADefaultPlayerController] LevelUpHandler: 찾은 Pet들 - %s"), *Pet->GetName());
			}
		}

		// 랜덤 Item 가져오기
		UWorld* World = GetWorld();
		if (!World) return;
		UItemPoolSubsystem* ItemPool = World->GetGameInstance()->GetSubsystem<UItemPoolSubsystem>(); /// GameInstance에서 ItemPoolSubsystem을 가져옴
		if (!ItemPool) return;
		TArray<TObjectPtr<UObject>> RandomItems = ItemPool->GetRandomItemObjects(3); /// 레벨업 보상으로 3개의 랜덤 아이템을 가져옴
		
		LevelUpData.RandomItemList = RandomItems;

		UE_LOG(LogTemp, Warning, TEXT("[ADefaultPlayerController] LevelUpHandler: 가져온 랜덤 아이템 개수 - %d"), RandomItems.Num());
		for (TObjectPtr<UObject> ItemObject : RandomItems)
		{
			UItemDataObject* ItemDataObject = Cast<UItemDataObject>(ItemObject);
			if (ItemDataObject)
			{
				FText ItemName = ItemDataObject->ItemData.ItemText.Name;
				UE_LOG(LogTemp, Warning, TEXT("[ADefaultPlayerController] LevelUpHandler: 가져온 랜덤 아이템 - %s"), *ItemName.ToString());
				//LevelUpData.RewardItems.Add(ItemData);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[ADefaultPlayerController] LevelUpHandler: 가져온 랜덤 아이템 객체가 UItemDataObject로 캐스팅되지 않았습니다."));
			}
		}


		// Level Up Widget 생성 및 화면에 추가
		ULevelUpRewardWidget* LevelUpWidget = CreateWidget<ULevelUpRewardWidget>(GetWorld(), LevelUpWidgetClass);
		UE_LOG(LogTemp, Warning, TEXT("[ADefaultPlayerController] LevelUpHandler called. LevelUpWidget created: %s"), *GetNameSafe(LevelUpWidget));

		if (LevelUpWidget)
		{
			LevelUpWidget->InitializeWithLevelUpData(LevelUpData); /// 레벨업 보상 창에 데이터 전달

			FInputModeGameAndUI InputModeData;

			// 위젯을 포커스하되, 마우스 클릭 시 게임 뷰포트가 마우스를 뺏어가지 않도록 설정
			InputModeData.SetWidgetToFocus(LevelUpWidget->TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			InputModeData.SetHideCursorDuringCapture(false); // 중요: 클릭 시 커서가 사라지지 않게 함

			this->SetInputMode(InputModeData);
			this->bShowMouseCursor = true; // 마우스 커서 표시

			LevelUpWidget->AddToViewport(1);

			UE_LOG(LogTemp, Warning, TEXT("[ADefaultPlayerController] 레벨업 창 활성화"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ADefaultPlayerController] LevelUpHandler called but LevelUpWidgetClass is not set."));
	}
}
