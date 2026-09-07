
#include "Core/BornToEndureGameInstance.h"
#include "Subsystem/ItemPoolSubsystem.h"
#include "Engine/DataTable.h"

void UBornToEndureGameInstance::Init()
{
	Super::Init();

	UItemPoolSubsystem* ItemPool = GetSubsystem<UItemPoolSubsystem>();
	check(ItemPool);

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
