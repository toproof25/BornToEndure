/**
 * @file ItemStoreWidget.h
 * @brief 상점 상품과 재화를 표시하고 사용자 요청을 컨트롤러에 전달하는 위젯입니다.
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/DataTableRow/ItemDataRow.h"
#include "ItemStoreWidget.generated.h"

class UPlayerExperienceComponent;
class UItemStoreEntryWidget;
class UPetItemTooltipWidget;
class UHorizontalBox;
class UButton;
class UTextBlock;
class UPetEntryWidget;
class APetCompanionCharacter;

UCLASS()
class BORNTOENDURE_API UItemStoreWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 골드 알림 구독 대상을 교체하고 현재 잔액을 표시합니다.
	 * @param Component 새 골드 컴포넌트. nullptr이면 기존 구독만 해제합니다.
	 */
	void BindCurrency(UPlayerExperienceComponent* Component);

	/**
	 * 기존 상품을 정리하고 새 상품 목록을 표시합니다.
	 * @param Offers 표시할 상품 핸들 배열입니다.
	 * @param Prices Offers와 같은 길이이며 같은 인덱스에 대응하는 골드 가격입니다.
	 * @return 필수 패널·클래스 누락, 배열 길이 불일치 또는 Entry 생성 실패 시 false입니다.
	 * @note 검증 전 기존 목록을 지우며, 생성 실패 시 부분 생성 목록도 정리합니다.
	 */
	bool DisplayOffers(const TArray<FItemDataHandle>& Offers, const TArray<int32>& Prices);

	/**
	 * 유효한 슬롯을 품절로 표시합니다.
	 * @param Index 표시 목록의 인덱스. 범위를 벗어나면 무시합니다.
	 */
	void MarkSold(int32 Index);

	/**
	 * 골드 구독과 상품·Pet 목록을 정리합니다. 반복 호출할 수 있습니다.
	 */
	void Shutdown();

	/**
	 * 유효한 Pet만 미선택 상태로 표시합니다.
	 * @return 필수 설정 누락 또는 생성 실패 시 false. 빈 목록은 허용합니다.
	 */
	bool DisplayPets(const TArray<TObjectPtr<APetCompanionCharacter>>& Pets);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

private:
	int32 SelectedItemIndex = INDEX_NONE;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> PetListBox;

	UPROPERTY(EditDefaultsOnly, Category = "Store")
	TSubclassOf<UPetEntryWidget> PetEntryWidgetClass;

	UPROPERTY()
	TArray<TObjectPtr<UPetEntryWidget>> PetEntries;

	void ClearPets();

	UFUNCTION()
	void HandlePetSelected(APetCompanionCharacter* Pet);

	UPROPERTY(EditDefaultsOnly, Category="Store")
	TSubclassOf<UItemStoreEntryWidget> EntryWidgetClass;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> GoldText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UHorizontalBox> ItemHorizontalBox;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> RerollButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPetItemTooltipWidget> ItemTooltipWidget;

	/** 현재 골드 구독 대상이며 재바인딩 또는 Shutdown에서 구독을 해제합니다. */
	UPROPERTY()
	TObjectPtr<UPlayerExperienceComponent> CurrencyComponent;

	/** 표시 중인 상품을 보관하며 ClearEntries에서 이벤트 연결과 함께 정리합니다. */
	UPROPERTY()
	TArray<TObjectPtr<UItemStoreEntryWidget>> Entries;

	void ClearEntries();
	void HandlePurchase(int32 Index);
	void HandleHover(int32 Index, const FItemDataHandle& Handle, bool bHovered);

	UFUNCTION()
	void HandleGoldChanged(int32 Gold);

	UFUNCTION()
	void HandleReroll();

	UFUNCTION()
	void HandleClose();
};
