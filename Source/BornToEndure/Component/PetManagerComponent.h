/**
 * @file PetManagerComponent.h
 * @brief 플레이어가 소유한 Pet들을 관리하는 컴포넌트 헤더
 * @author toproof (kmnlmn123@gmail.com)
 * @date 2026-04-20
 * @details 
 * - 이 파일은 플레이어가 소유한 Pet들을 관리하고, 아이템 획득 시 적절한 Pet에게 전달하는 기능을 정의
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Delegates/Delegate.h"
#include "PetManagerComponent.generated.h"

class APetCompanionCharacter;
class UPetItemDataAsset;

DECLARE_LOG_CATEGORY_EXTERN(LogPetManager, Log, All);

/**
 * @brief 플레이어가 소유한 Pet들을 관리하는 컴포넌트
 * @details 
 * - Pet 스폰 / 제거
 * - 아이템 획득 시 어떤 Pet에게 줄지 결정 및 전달
 * - 외부(아이템 픽업 액터, UI)로부터의 요청을 Pet에게 위임
 * - Pet과의 유일한 소통 수단으로 개별 Pet을 접근하지 않아도 되도록 설계
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BORNTOENDURE_API UPetManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPetManagerComponent();

protected:
	virtual void BeginPlay() override;

public:

    /**
     * @brief Pet을 관리 목록에 추가한 후 레벨에 스폰하는 함수
     * @param PetClassOverride null이면 DefaultPetClass를 사용
     * @return 스폰된 Pet의 포인터
     * @details
	 * - Pet을 성공적으로 스폰한 후 FOnPetAdded Delegate를 호출
     */
    UFUNCTION(BlueprintCallable, Category = "Pet")
    APetCompanionCharacter* SpawnAndAddPet(TSubclassOf<APetCompanionCharacter> PetClassOverride = nullptr);

    /**
	 * @brief Pet을 관리 목록에서 제거하고, Destroy하는 함수
     * @param PetToRemove 제거하고자 하는 Pet의 포인터
	 * @details
     * - Pet을 성공적으로 제거한 후 FOnPetRemoved Delegate를 호출
     */
    UFUNCTION(BlueprintCallable, Category = "Pet")
    void RemovePet(APetCompanionCharacter* PetToRemove);

    /**
     * @brief UI등에서 아이템을 선택한 후 아이템의 DataAsset을 로드하여 인스턴스화하는 함수
	 * @param ItemAssetId UI에서 선택하여 고른 아이템의 PrimaryAssetId
	 * @details
	 * - 로드가 안된 아이템은 비동기 로드 후 OnItemDataLoaded 콜백에서 Pet에게 전달한다.
     * - 아이템 추가는 해당 함수를 시작으로 비동기 로드 -> Pet 선택 SelectPetForItem -> GiveItemToPet으로 전달된다
     */
    UFUNCTION(BlueprintCallable, Category = "Pet|Item")
    void RequestItemForPet(const FPrimaryAssetId& ItemAssetId);

    /**
     * @brief 아이템을 선택한 Pet에게 전달하는 함수
     * @param TargetPet 아이템을 주고자 하는 Pet의 포인터
     * @param ItemData 아이템의 DataAsset 포인터
     * @todo
     * - 현재는 기본만 구현되어 있으며, 추후 변경 예정
     */
    UFUNCTION(BlueprintCallable, Category = "Pet|Item")
    void GiveItemToPet(APetCompanionCharacter* TargetPet, UPetItemDataAsset* ItemData);

    /**
     * @brief Pet 소유한 아이템을 제거하는 함수
     * @param TargetPet 아이템을 소유한 Pet 포인터
	 * @param InstanceId 제거하고자 하는 아이템 인스턴스의 GUID
	 * @details
	 * - UI 혹은 외부에서 아이템 제거를 요청할 때 사용한다. 
     * - 아이템 제거는 PetItemComponent의 RemoveItem을 호출하는 형태로 구현
     */
    UFUNCTION(BlueprintCallable, Category = "Pet|Item")
    void RemoveItemFromPet(APetCompanionCharacter* TargetPet, const FGuid& InstanceId);


    /**
     * @brief 현재 보유한 Pet List를 반환하는 함수
	 * @return Player가 소유한 Pet들의 포인터 배열
     */
    const TArray<TObjectPtr<APetCompanionCharacter>>& GetPetList() const { return PetList; } //UFUNCTION(BlueprintPure, Category = "Pet")

    /**
     * @brief 현재 활성화 된 Pet을 반환하는 함수
     * @return 활성화 된 Pet의 포인터, 없으면 nullptr
     * @todo
     * - 현재는 Pet이 하나로 0번째 Pet만 반환함
     * - 추후 UI 등에서 활성화된 Pet을 선택할 수 있도록 구현할 예정
     */
    UFUNCTION(BlueprintPure, Category = "Pet")
    APetCompanionCharacter* GetActivePet() const;

    UFUNCTION(BlueprintPure, Category = "Pet")
    int32 GetPetCount() const { return PetList.Num(); }

    // Pet이 추가될 때 방송한다
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnPetAdded, APetCompanionCharacter*);
    FOnPetAdded OnPetAdded;

    // Pet이 제거될 때 방송한다
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnPetRemoved, APetCompanionCharacter*);
    FOnPetRemoved OnPetRemoved;

    // Pet이 아이템을 받을 때 혹은 제거될 때 방송한다
    DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPetItemReceived, APetCompanionCharacter*, const UPetItemDataAsset*);
    FOnPetItemReceived OnPetItemReceived;


public:

    UPROPERTY(EditDefaultsOnly, Category = "Pet|Config")
    TSubclassOf<APetCompanionCharacter> DefaultPetClass;

    UPROPERTY(EditDefaultsOnly, Category = "Pet|Config")
    float SpawnOffsetDistance = 120.f;

private:
    UPROPERTY(VisibleAnywhere, Category = "Pet")
    TArray<TObjectPtr<APetCompanionCharacter>> PetList;

    FTransform CalculateSpawnTransform(int32 PetIndex) const;

    /**
     * @brief 아이템을 전달할 Pet을 선택하는 함수
	 * @param ItemData 전달할 Item의 DataAsset 포인터
     * @return 선택된 Pet의 포인터, 없으면 nullptr
     * @todo
     * - 현재는 GetActivePet()와 마찬가지로 0번재 Pet만 반환. 추후 UI를 구성하여 선택하도록 구현
     * - Selection Policy 패턴으로 확장
     */
    APetCompanionCharacter* SelectPetForItem(const UPetItemDataAsset* ItemData) const;

    void OnItemDataLoaded(FPrimaryAssetId ItemAssetId);

};