// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ImGuiDelegates.h"
#include "GameDebugActor.generated.h"

class ABornToEndureGameModeBase;

UCLASS()
class BORNTOENDURE_API AGameDebugActor : public AActor
{
	GENERATED_BODY()

public:
	AGameDebugActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
#if !UE_BUILD_SHIPPING
	// Debug requests only; this is not a copy of the game's runtime state.
	enum class EGameDebugAction : uint8
	{
		None,
		StartGame,
		SpawnWave,
		EndGame
	};

	void DrawDebugWindow();
	EGameDebugAction DrawGameFlowTab(ABornToEndureGameModeBase* GameMode);
	void ExecuteGameAction(EGameDebugAction Action);

	FImGuiDelegateHandle ImGuiDelegateHandle;
	FString WindowTitle;
	FString LastActionMessage;
#endif

};
