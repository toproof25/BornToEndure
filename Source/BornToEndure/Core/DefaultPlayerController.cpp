/**
 * @file DefaultPlayerController.cpp
 * @brief 플레이어 입력과 상점 구매 및 UI 전환을 처리합니다.
 */

#include "Core/DefaultPlayerController.h"
#include "UI/ItemStoreWidget.h"

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

	//SetPause(bInPauseGame);
}

void ADefaultPlayerController::SetGameAndUIInputMode(UUserWidget* WidgetToFocus, bool bIgnoreMoveInput, bool bIgnoreLookInput, bool bInPauseGame)
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

	SetPause(bInPauseGame);
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

void ADefaultPlayerController::HandleTabToggleInput()
{
	if (bStoreOpen) return;
	bool bIsPetDetailWidgetVisible = PlayerHUDWidgetInstance->TogglePetDetailWidget();
	if (bIsPetDetailWidgetVisible) SetGameAndUIInputMode(PlayerHUDWidgetInstance, true, false, true);
	else SetGameInputMode();
	UE_LOG(LogTemp, Log, TEXT("[ADefaultPlayerController] TabToggle Input Handled: PetDetailWidget visibility toggled."));
}

void ADefaultPlayerController::HandleMerchantInteraction()
{
	if (IsPaused() || bStoreOpen || !IsLocalController() || !ItemStoreWidgetClass)
	{
		return;
	}

	ACombatPlayerState* PS = GetPlayerState<ACombatPlayerState>();
	PlayerExpComp = PS ? PS->GetPlayerExperienceComponent() : nullptr;
	if (!PlayerExpComp)
	{
		return;
	}

	ItemStoreWidgetInstance = CreateWidget<UItemStoreWidget>(this, ItemStoreWidgetClass);
	if (!ItemStoreWidgetInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ADefaultPlayerController] ItemStoreWidgetInstance 생성 실패"));
		return;
	}

	bStoreOpen = true;
	SelectedStorePet.Reset();
	ItemStoreWidgetInstance->BindCurrency(PlayerExpComp);

	UPetManagerComponent* PetManager = GetPawn() ? GetPawn()->FindComponentByClass<UPetManagerComponent>() : nullptr;
	const TArray<TObjectPtr<APetCompanionCharacter>> Pets = PetManager ? PetManager->GetPetList() : TArray<TObjectPtr<APetCompanionCharacter>>();
	if (!ItemStoreWidgetInstance->DisplayPets(Pets))
	{
		UE_LOG(LogTemp, Warning, TEXT("[ADefaultPlayerController] 상점 Pet 목록 표시 실패: 위젯 설정을 확인하세요."));
		CloseItemStore();
		return;
	}

	RerollStoreItems();
	if (!bStoreOpen)
	{
		return;
	}

	ItemStoreWidgetInstance->AddToViewport(50);
	SetGameAndUIInputMode(ItemStoreWidgetInstance, true, true, false);
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
	CloseItemStore(false);
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
	CloseItemStore(false);
	UE_LOG(LogTemp, Warning, TEXT("[ADefaultPlayerController] Player has died. Handling death."));
	// 플레이어가 죽었을 때 처리할 로직을 여기에 추가
	// 예: 게임 오버 화면 표시, 리스폰 처리 등
}

void ADefaultPlayerController::LevelUpHandler(int32 NewLevel)
{
	CloseItemStore();
	// Pet 목록 가져오기
	FLevelUpDataBundle LevelUpData;
	UPetManagerComponent* PetManager = GetPawn()->FindComponentByClass<UPetManagerComponent>();
	LevelUpData.PetList = PetManager ? PetManager->GetPetList() : TArray<TObjectPtr<APetCompanionCharacter>>(); 

	// 랜덤 Item 가져오기
	UWorld* World = GetWorld();
	if (!World) return;
	UItemPoolSubsystem* ItemPool = World->GetGameInstance()->GetSubsystem<UItemPoolSubsystem>(); /// GameInstance에서 ItemPoolSubsystem을 가져옴
	if (!ItemPool) return;
	LevelUpData.RandomItemHandles = ItemPool->GetRandomItemData(3); /// 레벨업 보상으로 3개의 랜덤 아이템을 가져옴
	UE_LOG(LogTemp, Warning, TEXT("[ADefaultPlayerController] LevelUpHandler: 가져온 랜덤 아이템 개수 - %d"), LevelUpData.RandomItemHandles.Num());

	PlayerHUDWidgetInstance->ShowLevelUpWidget(LevelUpData); /// PlayerHUDWidget에 레벨업 보상 창 활성화 요청
}

void ADefaultPlayerController::RerollStoreItems()
{
	if (!bStoreOpen || bStorePurchaseInProgress || !ItemStoreWidgetInstance)
	{
		return;
	}

	UItemPoolSubsystem* Pool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UItemPoolSubsystem>() : nullptr;
	if (!Pool || !Pool->WeaponDataTable)
	{
		CloseItemStore();
		return;
	}

	TArray<FItemDataHandle> Offers;
	TArray<int32> Prices;
	StoreItemIDs.Empty();
	StoreSold.Empty();

	for (const FItemDataHandle& Handle : Pool->GetRandomWeaponItemData(StoreOfferCount))
	{
		const FWeaponItemDataRow* Row = Pool->GetWeaponItemDataRowByID(Handle.ItemRowName);
		if (!Row || Handle.ItemType != EItemType::Weapon || Row->ItemCost < 0)
		{
			continue;
		}

		Offers.Add(Handle);
		Prices.Add(Row->ItemCost);
		StoreItemIDs.Add(Handle.ItemRowName);
		StoreSold.Add(false);
	}

	if (Offers.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[ADefaultPlayerController] 상점 상품 표시 실패: 무기 데이터가 없습니다."));
		CloseItemStore();
		return;
	}

	if (!ItemStoreWidgetInstance->DisplayOffers(Offers, Prices))
	{
		UE_LOG(LogTemp, Warning, TEXT("[ADefaultPlayerController] 상점 상품 표시 실패: !ItemStoreWidgetInstance->DisplayOffers(Offers, Prices) 무기 데이터와 위젯 클래스 설정을 확인하세요."));
		CloseItemStore();
	}


}

void ADefaultPlayerController::PurchaseStoreItem(int32 Index)
{
	if (!bStoreOpen || bStorePurchaseInProgress || !PlayerExpComp ||
		!StoreItemIDs.IsValidIndex(Index) || !StoreSold.IsValidIndex(Index) || StoreSold[Index])
	{
		return;
	}

	APetCompanionCharacter* Pet = SelectedStorePet.Get();
	UPetManagerComponent* PetManager = GetPawn() ? GetPawn()->FindComponentByClass<UPetManagerComponent>() : nullptr;
	if (!IsValid(Pet) || !PetManager || !PetManager->GetPetList().Contains(Pet))
	{
		return;
	}

	UItemPoolSubsystem* Pool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UItemPoolSubsystem>() : nullptr;
	const FWeaponItemDataRow* Row = Pool ? Pool->GetWeaponItemDataRowByID(StoreItemIDs[Index]) : nullptr;
	if (!Row || Row->ItemCost < 0)
	{
		return;
	}

	// 골드 변경 알림이 동기 방송되는 동안 재구매 요청을 차단합니다.
	TGuardValue<bool> PurchaseGuard(bStorePurchaseInProgress, true);
	const FName PurchasedID = StoreItemIDs[Index];
	const int32 Cost = Row->ItemCost;
	const FString PetName = Pet->GetPetName().ToString();
	const FString PetActorName = Pet->GetName();
	if (!PlayerExpComp->TrySpendGold(Cost))
	{
		return;
	}
	if (StoreSold.IsValidIndex(Index))
	{
		StoreSold[Index] = true;
	}
	if (ItemStoreWidgetInstance)
	{
		ItemStoreWidgetInstance->MarkSold(Index);
	}

	PetManager->GiveItemToPet(Pet, FItemDataHandle(EItemType::Weapon, PurchasedID));

	UE_LOG(LogTemp, Log, TEXT("[ADefaultPlayerController] 상점 구매: Pet=%s (%s), 아이템=%s, 비용=%d, 남은 골드=%d"),
		*PetName, *PetActorName, *PurchasedID.ToString(), Cost, PlayerExpComp->GetCurrentGold());
}

void ADefaultPlayerController::CloseItemStore(bool bRestoreInput)
{
	const bool bWasOpen = bStoreOpen;
	bStoreOpen = false;
	SelectedStorePet.Reset();
	if (ItemStoreWidgetInstance)
	{
		ItemStoreWidgetInstance->Shutdown();
		ItemStoreWidgetInstance->RemoveFromParent();
		ItemStoreWidgetInstance = nullptr;
	}

	StoreItemIDs.Empty();
	StoreSold.Empty();
	if (bWasOpen && bRestoreInput)
	{
		RestoreGameInputMode();
	}
}

bool ADefaultPlayerController::SelectStorePet(APetCompanionCharacter* Pet)
{
	UPetManagerComponent* PetManager = GetPawn() ? GetPawn()->FindComponentByClass<UPetManagerComponent>() : nullptr;
	if (!bStoreOpen || !IsValid(Pet) || !PetManager || !PetManager->GetPetList().Contains(Pet))
	{
		return false;
	}

	SelectedStorePet = Pet;
	return true;
}
