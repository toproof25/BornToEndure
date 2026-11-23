/**
 * @file PetManagerComponent.cpp
 * @brief 플레이어의 Pet 스폰과 소유 목록 및 Pet 아이템 전달을 관리합니다.
 */
#include "Component/PetManagerComponent.h"

#include "Character/Pet/PetCompanionCharacter.h"
#include "Data/PetItemDataAsset.h"
#include "Engine/AssetManager.h"
#include "Component/PetItemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Data/DataTableRow/ItemDataRow.h"
#include "Data/DataTableRow/StatItemDataRow.h"
#include "Data/DataTableRow/PetDataRow.h"

#include "Data/PetBaseDataAsset.h"
#include "Subsystem/ItemPoolSubsystem.h"

DEFINE_LOG_CATEGORY(LogPetManager);

UPetManagerComponent::UPetManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UPetManagerComponent::BeginPlay()
{
	Super::BeginPlay();

}

APetCompanionCharacter* UPetManagerComponent::SpawnAndAddPet(FName RowName, TSoftObjectPtr<UPetBaseDataAsset> PetDataAsset)
{
	UWorld* World = GetWorld();
	if (!World || !IsValid(DefaultPetClass.Get()))
	{
		UE_LOG(LogPetManager, Warning, TEXT("[UPetManagerComponent] Pet을 스폰할 월드 또는 클래스가 없습니다: %s"), *RowName.ToString());
		return nullptr;
	}

	UPetBaseDataAsset* LoadedPetData = PetDataAsset.LoadSynchronous();
	if (!IsValid(LoadedPetData))
	{
		UE_LOG(LogPetManager, Warning, TEXT("[UPetManagerComponent] Pet DataAsset 로드 실패: %s"), *RowName.ToString());
		return nullptr;
	}

	const FTransform SpawnTransform = CalculateSpawnTransform(PetList.Num());
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APetCompanionCharacter* NewPet = World->SpawnActor<APetCompanionCharacter>(DefaultPetClass, SpawnTransform, SpawnParams);
	if (!IsValid(NewPet))
	{
		UE_LOG(LogPetManager, Warning, TEXT("[UPetManagerComponent] Pet 스폰 실패: %s"), *RowName.ToString());
		return nullptr;
	}

	NewPet->InitializeFromDataAsset(RowName, LoadedPetData);
	PetList.Add(NewPet);

	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		NewPet->SetFollowOwner(OwnerPawn);
	}

	UE_LOG(LogPetManager, Log, TEXT("[UPetManagerComponent] Pet 추가 완료: %s (보유 %d)"), *RowName.ToString(), PetList.Num());
	OnPetAdded.Broadcast(this);
	return NewPet;
}

void UPetManagerComponent::RemovePet(APetCompanionCharacter* PetToRemove)
{
    if (!PetToRemove) return;

    PetList.RemoveSingle(PetToRemove);
    OnPetRemoved.Broadcast(this);
    PetToRemove->Destroy();
}

/*
void UPetManagerComponent::RequestItemForPet(const FPrimaryAssetId& ItemAssetId)
{
    if (!ItemAssetId.IsValid())
    {
        UE_LOG(LogPetManager, Warning, TEXT("[PetManagerComponent] Invalid ItemAssetId."));
        return;
    }

    UAssetManager& AM = UAssetManager::Get();

    // 아이템 DataAsset을 비동기 로드하며, 로드 후 OnItemDataLoaded에서 Pet을 선택 및 아이템을 전달한다
    AM.GetStreamableManager().RequestAsyncLoad(
        AM.GetPrimaryAssetPath(ItemAssetId),
        FStreamableDelegate::CreateUObject(this, &UPetManagerComponent::OnItemDataLoaded, ItemAssetId)
    );

    UE_LOG(LogPetManager, Verbose, TEXT("[PetManagerComponent] Async loading item: %s"), *ItemAssetId.ToString());
}
*/
void UPetManagerComponent::GiveItemToPet(APetCompanionCharacter* TargetPet, FItemDataHandle ItemData)
{
	if (!TargetPet || ItemData.ItemRowName.IsNone()) return;

	if (!PetList.Contains(TargetPet))
	{
		UE_LOG(LogPetManager, Warning, TEXT("[PetManagerComponent] Target pet is not owned."));
		return;
	}

	UPetItemComponent* ItemComp = TargetPet->GetItemComponent();
	if (ItemComp)
	{
		ItemComp->AddItem(ItemData);
		OnPetItemReceived.Broadcast(TargetPet);
	}
}

void UPetManagerComponent::RemoveItemFromPet(APetCompanionCharacter* TargetPet, const FGuid& InstanceId)
{
    if (!TargetPet || !InstanceId.IsValid()) return;
    if (!PetList.Contains(TargetPet))
    {
        UE_LOG(LogPetManager, Warning, TEXT("[PetManagerComponent] Target pet is not owned."));
        return;
    }

    UPetItemComponent* ItemComp = TargetPet->GetItemComponent();
    if (ItemComp)
    {
		ItemComp->RemoveItem(InstanceId);
        OnPetItemReceived.Broadcast(TargetPet);
    }
}

APetCompanionCharacter* UPetManagerComponent::GetActivePet() const
{
    // 현재 구현: 첫 번째 Pet이 Active Pet
    // 추후 다중 Pet 선택 시스템으로 확장 가능
    return PetList.IsEmpty() ? nullptr : PetList[0].Get();
}

FTransform UPetManagerComponent::CalculateSpawnTransform(int32 PetIndex) const
{
    AActor* Owner = GetOwner();
    if (!Owner) return FTransform::Identity;

    // 플레이어 뒤쪽에 Pet 인덱스 기반으로 약간씩 옆으로 배치
    const FVector OwnerLocation = Owner->GetActorLocation();
    const FVector BackDirection = -Owner->GetActorForwardVector();
    const FVector SideOffset = Owner->GetActorRightVector() * (PetIndex * 80.f);

    const FVector SpawnLocation = OwnerLocation
        + BackDirection * SpawnOffsetDistance
        + SideOffset
        + FVector(0.f, 0.f, 10.f); // 바닥에서 약간 위

    return FTransform(Owner->GetActorRotation(), SpawnLocation);
}

APetCompanionCharacter* UPetManagerComponent::SelectPetForItem(
    const UPetItemDataAsset* ItemData) const
{
    // [현재 구현] 단순하게 첫 번째 Pet 선택
    // [추후 확장] Selection Policy 인터페이스 주입으로 전략 교체 가능
    // 예: 가장 아이템이 적은 Pet, 플레이어가 선택한 Pet 등
    return GetActivePet();
}
