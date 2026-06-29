
#include "Core/BornToEndureGameInstance.h"
#include "Subsystem/ItemPoolSubsystem.h"
#include "Engine/DataTable.h"

void UBornToEndureGameInstance::Init()
{
	Super::Init();

	UItemPoolSubsystem* ItemPool = GetSubsystem<UItemPoolSubsystem>();

	if (ItemPool && ItemDataTableClass)
	{
		ItemPool->InitializeItemPoolSubsystem(ItemDataTableClass);
		UE_LOG(LogTemp, Log, TEXT("GameInstance: 아이템 데이터 테이블을 서브시스템으로 전달 완료!"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameInstance: MasterItemDataTable이 설정되지 않았습니다!"));
	}
}
