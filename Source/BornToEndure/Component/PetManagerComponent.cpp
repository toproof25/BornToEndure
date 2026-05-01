#include "Component/PetManagerComponent.h"

#include "Character/Pet/PetCompanionCharacter.h"
#include "Data/PetItemDataAsset.h"
#include "Engine/AssetManager.h"
#include "Component/PetItemComponent.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogPetManager);

UPetManagerComponent::UPetManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UPetManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogPetManager, Display, TEXT("PetManager BeginPlay called. Attempting to spawn pet."));

    // DefaultPetClass가 설정되어 있으면 게임 시작 시 자동 스폰
    if (DefaultPetClass)
    {
        SpawnAndAddPet(DefaultPetClass);
    }
    else
    {
        UE_LOG(LogPetManager, Warning, TEXT("[PetManagerComponent] DefaultPetClass is not set."));
    }

	
}

APetCompanionCharacter* UPetManagerComponent::SpawnAndAddPet(TSubclassOf<APetCompanionCharacter> PetClassOverride)
{
    // 파라미터로 들어온 Pet이 유효하면 사용하고 아니면 기본 Pet을 사용한다
    TSubclassOf<APetCompanionCharacter> ClassToSpawn = PetClassOverride ? PetClassOverride : DefaultPetClass;

    if (!ClassToSpawn)
    {
        UE_LOG(LogPetManager, Error, TEXT("[PetManagerComponent] No pet class to spawn."));
        return nullptr;
    }

    UWorld* World = GetWorld();
    if (!World) return nullptr;

    // 플레이어 근방에 Pet을 스폰
    const FTransform SpawnTransform = CalculateSpawnTransform(PetList.Num());
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = GetOwner();
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    APetCompanionCharacter* NewPet = World->SpawnActor<APetCompanionCharacter>(ClassToSpawn, SpawnTransform, SpawnParams);

    if (!NewPet)
    {
        UE_LOG(LogPetManager, Error, TEXT("[PetManagerComponent] Pet spawn failed."));
        return nullptr;
    }

    // Pet 관리 목록에 추가
    PetList.Add(NewPet);

    // Pet의 주인(플레이어)을 설정
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        NewPet->SetFollowOwner(OwnerPawn);
    }

    // Pet 추가 이벤트를 방송
    OnPetAdded.Broadcast(NewPet);
    UE_LOG(LogPetManager, Log, TEXT("[PetManagerComponent] Pet spawned: %s (Total: %d)"),*NewPet->GetName(), PetList.Num());

    return NewPet;
}

void UPetManagerComponent::RemovePet(APetCompanionCharacter* PetToRemove)
{
    if (!PetToRemove) return;

    PetList.RemoveSingle(PetToRemove);
    OnPetRemoved.Broadcast(PetToRemove);
    PetToRemove->Destroy();
}

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

void UPetManagerComponent::GiveItemToPet(APetCompanionCharacter* TargetPet, UPetItemDataAsset* ItemData)
{
    if (!TargetPet || !ItemData) return;
    if (!PetList.Contains(TargetPet))
    {
        UE_LOG(LogPetManager, Warning, TEXT("[PetManagerComponent] Target pet is not owned."));
        return;
    }

    UPetItemComponent* ItemComp = TargetPet->GetItemComponent();
    if (ItemComp)
    {
        ItemComp->AddItem(ItemData);
        //OnPetItemReceived.Broadcast(TargetPet, ItemData);
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
        //OnPetItemReceived.Broadcast(TargetPet, nullptr);
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

void UPetManagerComponent::OnItemDataLoaded(FPrimaryAssetId ItemAssetId)
{
    UAssetManager& AM = UAssetManager::Get();
    UPetItemDataAsset* ItemData = Cast<UPetItemDataAsset>(AM.GetPrimaryAssetObject(ItemAssetId));

    if (!ItemData)
    {
        UE_LOG(LogPetManager, Error,
            TEXT("[PetManagerComponent] Failed to load item data: %s"), *ItemAssetId.ToString());
        return;
    }

    APetCompanionCharacter* SelectedPet = SelectPetForItem(ItemData);
    if (!SelectedPet)
    {
        UE_LOG(LogPetManager, Warning, TEXT("[PetManagerComponent] No valid pet to receive item."));
        return;
    }

    GiveItemToPet(SelectedPet, ItemData);

    UE_LOG(LogPetManager, Log, TEXT("[PetManagerComponent] Item '%s' given to pet '%s'"), *ItemData->ItemName.ToString(), *SelectedPet->GetName());
}