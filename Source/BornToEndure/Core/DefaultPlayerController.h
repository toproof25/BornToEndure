/**
* @file DefaultPlayerController.h
* @brief 플레이어 입력과 상점 구매 및 UI를 조정합니다.
* 
* - 플레이어 입력 처리 및 UI 관리
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "DefaultPlayerController.generated.h"

class UItemStoreWidget;
class APetCompanionCharacter;
class UInputMappingContext;
class UPlayerHUDWidget;
class ULevelUpRewardWidget;
class UPlayerExperienceComponent;
class UPlayerHealthBarWidget;
class UUserWidget;

UCLASS()
class BORNTOENDURE_API ADefaultPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	void SetGameInputMode();
	void SetUIInputMode(UUserWidget* WidgetToFocus, bool bInPauseGame = false);
	void SetGameAndUIInputMode(
		UUserWidget* WidgetToFocus,
		bool bIgnoreMoveInput = true,
		bool bIgnoreLookInput = false,
		bool bInPauseGame = false
	);
	void RestoreGameInputMode();

	void TravelToLobbyLevel();

	void HandleTabToggleInput();

	/**
	 * 상인 상호작용으로 상품을 추첨하고 상점을 엽니다.
	 * @note 일시정지·이미 열린 상점·비로컬 컨트롤러·필수 클래스나 컴포넌트 누락 시 열지 않습니다.
	 */
	void HandleMerchantInteraction();

	/**
	 * 현재 상품을 검증하고 골드 차감 성공 시 품절 처리와 구매 로그를 남깁니다.
	 * @param Index 현재 상점 상품 목록의 인덱스입니다.
	 * @note 닫힌 상점·재진입·잘못된 슬롯·품절·잘못된 데이터·Pet 미선택 또는 보유 해제·잔액 부족 시 구매하지 않습니다.
	 */
	void PurchaseStoreItem(int32 Index);

	/**
	 * 현재 보유한 Pet을 상점 전달 대상으로 선택합니다.
	 * @return 상점이 닫혔거나 Pet이 유효하지 않거나 현재 보유 목록에 없으면 false입니다.
	 */
	bool SelectStorePet(APetCompanionCharacter* Pet);


	/**
	 * 비용 없이 상품을 다시 추첨합니다.
	 * @note 상점이 닫혔거나 구매 중이면 무시하며, 풀·표시 실패 또는 빈 상품 목록이면 상점을 닫습니다.
	 */
	void RerollStoreItems();

	/**
	 * 상점과 상품 상태를 정리합니다.
	 * @param bRestoreInput true이면 열려 있던 상점의 게임 입력을 복구합니다. 사망·종료 시에는 false로 유지합니다.
	 */
	void CloseItemStore(bool bRestoreInput = true);

protected:

	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** * @brief Input Mapping Context를 편집기에서 할당 가능하도록 선언 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UI, meta = (AllowedClasses = "PlayerHUDWidget"))
	TSubclassOf<UPlayerHUDWidget> PlayerHUDWidgetClass;

private:
	/** 전달 대상은 소유하지 않으며 구매 직전에 현재 보유 여부를 재검증합니다. */
	TWeakObjectPtr<APetCompanionCharacter> SelectedStorePet;

	UPROPERTY(EditDefaultsOnly, Category="Store")
	TSubclassOf<UItemStoreWidget> ItemStoreWidgetClass;

	UPROPERTY()
	TObjectPtr<UItemStoreWidget> ItemStoreWidgetInstance;

	/** 상품 ID와 품절 상태는 같은 인덱스로 대응합니다. */
	TArray<FName> StoreItemIDs;
	TArray<bool> StoreSold;

	bool bStoreOpen = false;

	/** 골드 변경 알림 방송 중 구매 재진입을 방지합니다. */
	bool bStorePurchaseInProgress = false;

	static constexpr int32 StoreOfferCount = 3;

	UPROPERTY()
	TObjectPtr<UPlayerHUDWidget> PlayerHUDWidgetInstance;

	UPROPERTY()
	TObjectPtr<UPlayerExperienceComponent> PlayerExpComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Level", meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UWorld> LobbyLevel;

	void SetUpDelegates();
	void SetUpPlayerInputMode();
	void SetUpPlayerHUDWidget();

	void HandlePlayerDeath();

	UFUNCTION()
	void LevelUpHandler(int32 NewLevel);



};
