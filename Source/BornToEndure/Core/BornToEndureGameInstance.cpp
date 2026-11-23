/**
 * @file BornToEndureGameInstance.cpp
 * @brief 테이블 초기화와 로비에서 본 게임으로 넘길 스타트 Pet 선택을 관리합니다.
 */
#include "Core/BornToEndureGameInstance.h"
#include "Subsystem/ItemPoolSubsystem.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UObject/Package.h"

void UBornToEndureGameInstance::Init()
{
	Super::Init();

	if (GEngine)
	{
		GEngine->OnTravelFailure().AddUObject(this, &UBornToEndureGameInstance::HandleStarterPetTravelFailure);
	}

	UItemPoolSubsystem* ItemPool = GetSubsystem<UItemPoolSubsystem>();
	check(ItemPool);

	if (PetDataTableClass)
	{
		ItemPool->InitializePetDataTable(PetDataTableClass);
		UE_LOG(LogTemp, Log, TEXT("GameInstance: Pet 데이터 테이블을 서브시스템으로 전달 완료!"));
	}
	if (StatItemDataTableClass)
	{
		ItemPool->InitializeItemPoolSubsystem(StatItemDataTableClass, 0);
		UE_LOG(LogTemp, Log, TEXT("GameInstance: Stat 아이템 데이터 테이블을 서브시스템으로 전달 완료!"));
	}
	if (WeaponItemDataTableClass)
	{
		ItemPool->InitializeItemPoolSubsystem(WeaponItemDataTableClass, 1);
		UE_LOG(LogTemp, Log, TEXT("GameInstance: Weapon 아이템 데이터 테이블을 서브시스템으로 전달 완료!"));
	}
}

void UBornToEndureGameInstance::Shutdown()
{
	if (GEngine)
	{
		GEngine->OnTravelFailure().RemoveAll(this);
	}

	Super::Shutdown();
}

bool UBornToEndureGameInstance::SelectStarterPet(FName RowName)
{
	if (RowName.IsNone() || !PendingStarterPetMap.IsNone())
	{
		return false;
	}

	UItemPoolSubsystem* ItemPool = GetSubsystem<UItemPoolSubsystem>();
	const FPetDataRow* Row = ItemPool ? ItemPool->GetPetDataRowByID(RowName) : nullptr;
	if (!Row || Row->PetBaseDataAsset.IsNull())
	{
		return false;
	}

	SelectedStarterPetRowName = RowName;
	SelectedStarterPetRow = *Row;
	return true;
}

FName UBornToEndureGameInstance::GetSelectedStarterPetRowName() const
{
	return SelectedStarterPetRowName;
}

void UBornToEndureGameInstance::ResetStarterPetSelection()
{
	SelectedStarterPetRowName = NAME_None;
	SelectedStarterPetRow = FPetDataRow{};
	PendingStarterPetMap = NAME_None;
}

bool UBornToEndureGameInstance::QueueStarterPetTravel(FName MapPackageName)
{
	if (MapPackageName.IsNone() || SelectedStarterPetRowName.IsNone() || !PendingStarterPetMap.IsNone())
	{
		return false;
	}

	PendingStarterPetMap = MapPackageName;
	return true;
}

bool UBornToEndureGameInstance::TakePendingStarterPet(const UWorld* World, FName& OutRowName, FPetDataRow& OutRow)
{
	if (!World || PendingStarterPetMap.IsNone())
	{
		return false;
	}

	const FName CurrentMap(*UWorld::RemovePIEPrefix(World->GetPackage()->GetName()));
	if (CurrentMap != PendingStarterPetMap)
	{
		return false;
	}

	OutRowName = SelectedStarterPetRowName;
	OutRow = SelectedStarterPetRow;
	PendingStarterPetMap = NAME_None;
	return true;
}

void UBornToEndureGameInstance::HandleStarterPetTravelFailure(
	UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString)
{
	if (!World || World->GetGameInstance() != this || PendingStarterPetMap.IsNone())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[UBornToEndureGameInstance] 스타트 Pet 맵 이동 실패 (%d, %s): %s"),
		static_cast<int32>(FailureType), *PendingStarterPetMap.ToString(), *ErrorString);
	PendingStarterPetMap = NAME_None;
}
