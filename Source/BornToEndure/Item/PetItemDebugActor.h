#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/DataTableRow/ItemDataRow.h"
#if !UE_BUILD_SHIPPING
#include "ImGuiDelegates.h"
#endif
#include "PetItemDebugActor.generated.h"

class APetCompanionCharacter;
class UPetManagerComponent;
class UItemPoolSubsystem;
class UDataTable;
class UPetItemDataAsset;

UCLASS()
class BORNTOENDURE_API APetItemDebugActor : public AActor
{
	GENERATED_BODY()

public:
	APetItemDebugActor();
	void RenderImGui();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Preserve existing Blueprint assignments; the live ItemPool remains authoritative.
	UPROPERTY(EditAnywhere, Category = "Debug")
	TObjectPtr<UDataTable> StatItemDataTable;

	UPROPERTY(EditAnywhere, Category = "Debug")
	TObjectPtr<UDataTable> WeaponItemDataTable;

	// Keep explicitly loaded weapon assets alive for the debug actor's lifetime.
	UPROPERTY(Transient)
	TArray<TObjectPtr<UPetItemDataAsset>> LoadedWeaponItems;

#if !UE_BUILD_SHIPPING
	FImGuiDelegateHandle ImGuiDelegateHandle;
	TWeakObjectPtr<APetCompanionCharacter> SelectedPet;
	TWeakObjectPtr<UDataTable> ListedTable;
	TArray<FName> RowNames;
	char SearchText[128] = {};
	FString LastResult;
	bool bLastSucceeded = false;

	UPetManagerComponent* ResolvePetManager() const;
	UItemPoolSubsystem* ResolveItemPool() const;
	void DrawPetSelection(UPetManagerComponent* Manager);
	void DrawCatalog(UItemPoolSubsystem* Pool, EItemType Type);
	void GiveItem(EItemType Type, FName RowName);
#endif
};
