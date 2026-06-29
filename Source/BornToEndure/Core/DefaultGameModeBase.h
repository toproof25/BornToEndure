// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DefaultGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class BORNTOENDURE_API ADefaultGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ADefaultGameModeBase();

protected:
	virtual void BeginPlay() override;

private:

	// 게임 시작 시 필요한 초기화 작업을 수행하는 함수
	void InitializeGame();
};
