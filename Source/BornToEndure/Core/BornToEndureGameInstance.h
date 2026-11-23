/**
* @file BornToEndureGameInstance.h
* @date 2026-06-06
* @author toproof (kmnlmn123@gmail.com)
* @brief 데이터 테이블을 서브시스템에 연결하고 레벨 이동 간 스타트 Pet 선택을 보관합니다.
*/
#pragma once

#include "CoreMinimal.h"
#include "Data/DataTableRow/PetDataRow.h"
#include "Engine/EngineBaseTypes.h"
#include "Engine/GameInstance.h"
#include "BornToEndureGameInstance.generated.h"

class UDataTable;

UCLASS()
class BORNTOENDURE_API UBornToEndureGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;
	virtual void Shutdown() override;

	/** @return 유효한 행을 저장했으면 true입니다. 이동이 예약된 동안에는 선택을 유지합니다. */
	bool SelectStarterPet(FName RowName);
	FName GetSelectedStarterPetRowName() const;
	void ResetStarterPetSelection();

	/** @param MapPackageName 이동할 맵의 전체 패키지 이름입니다. */
	bool QueueStarterPetTravel(FName MapPackageName);

	/** @return 예약된 맵에 도착해 선택을 한 번 소비했으면 true입니다. */
	bool TakePendingStarterPet(const UWorld* World, FName& OutRowName, FPetDataRow& OutRow);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataTable|Pet")
	TObjectPtr<UDataTable> PetDataTableClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataTable|Item")
	TObjectPtr<UDataTable> StatItemDataTableClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataTable|Item")
	TObjectPtr<UDataTable> WeaponItemDataTableClass;

private:
	void HandleStarterPetTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString);

	FName SelectedStarterPetRowName = NAME_None;
	FPetDataRow SelectedStarterPetRow{};
	FName PendingStarterPetMap = NAME_None;
};
