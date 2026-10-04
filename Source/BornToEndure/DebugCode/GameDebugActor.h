/**
* @file GameDebugActor.h
* @brief 게임의 전체 흐름을 디버깅 및 테스트를 위한 액터
*/
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
		StartWave,
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
