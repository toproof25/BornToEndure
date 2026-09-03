/**
* @file ItemPoolSubsystem.h
* @date 2026-06-06
* @modified 2026-06-06
* @author toproof (kmnlmn123@gmail.com)
* @brief BornToEndureGameInstance.h 게임 인스턴스 클래스, 게임 전반적인 상태와 서브시스템 관리
* - ItemPoolSubsystem 초기화 및 관리에 활용 (아이템 DataTable을 전달하기 위해 처음 생성된 클래스)
*/
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BornToEndureGameInstance.generated.h"

class UDataTable;

UCLASS()
class BORNTOENDURE_API UBornToEndureGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<UDataTable> ItemDataTableClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<UDataTable> StatItemDataTableClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<UDataTable> WeaponItemDataTableClass;
};
