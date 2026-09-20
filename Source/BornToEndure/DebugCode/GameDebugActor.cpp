// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugCode/GameDebugActor.h"

#if !UE_BUILD_SHIPPING
#include "Core/BornToEndureGameModeBase.h"
#include "Engine/World.h"
#include "ImGuiModule.h"
#include "imgui.h"
#endif

AGameDebugActor::AGameDebugActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AGameDebugActor::BeginPlay()
{
	Super::BeginPlay();

#if !UE_BUILD_SHIPPING
	WindowTitle = FString::Printf(TEXT("게임 종합 디버그 | %s###GameDebug_%s"), *GetName(), *GetPathName());
	LastActionMessage = TEXT("아직 호출한 함수가 없습니다.");
	if (FImGuiModule::IsAvailable() && IsValid(GetWorld()))
	{
		const FImGuiDelegate Delegate = FImGuiDelegate::CreateUObject(this, &AGameDebugActor::DrawDebugWindow);
		ImGuiDelegateHandle = FImGuiModule::Get().AddWorldImGuiDelegate(GetWorld(), Delegate);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameDebugActor] Cannot register ImGui: module or world unavailable."));
	}
#endif
}

void AGameDebugActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
#if !UE_BUILD_SHIPPING
	if (FImGuiModule::IsAvailable() && ImGuiDelegateHandle.IsValid())
	{
		FImGuiModule::Get().RemoveImGuiDelegate(ImGuiDelegateHandle);
	}
	ImGuiDelegateHandle.Reset();
	WindowTitle.Reset();
	LastActionMessage.Reset();
#endif
	Super::EndPlay(EndPlayReason);
}

#if !UE_BUILD_SHIPPING
void AGameDebugActor::DrawDebugWindow()
{
	UWorld* World = GetWorld();
	if (!IsValid(World) || IsActorBeingDestroyed() || ImGui::GetCurrentContext() == nullptr)
	{
		return;
	}

	EGameDebugAction RequestedAction = EGameDebugAction::None;
	ImGui::SetNextWindowSize(ImVec2(580.0f, 370.0f), ImGuiCond_FirstUseEver);
	if (ImGui::Begin(TCHAR_TO_UTF8(*WindowTitle)))
	{
		ImGui::TextUnformatted("게임 진행 테스트 및 상태 확인");
		ImGui::TextDisabled("마우스 입력: 콘솔에서 ImGui.InputEnabled 1 (해제: 0)");
		ImGui::Separator();
		if (ImGui::BeginTabBar("GameDebugTabs"))
		{
			if (ImGui::BeginTabItem("게임 진행"))
			{
				RequestedAction = DrawGameFlowTab(World->GetAuthGameMode<ABornToEndureGameModeBase>());
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::End();

	// Finish all ImGui stacks before invoking a game operation.
	if (RequestedAction != EGameDebugAction::None)
	{
		ExecuteGameAction(RequestedAction);
	}
}

AGameDebugActor::EGameDebugAction AGameDebugActor::DrawGameFlowTab(ABornToEndureGameModeBase* GameMode)
{
	const bool bCanCall = IsValid(GameMode) && !GameMode->IsActorBeingDestroyed() && GameMode->HasActorBegunPlay();
	ImGui::Text("월드: %s", TCHAR_TO_UTF8(*GetWorld()->GetName()));
	if (bCanCall)
	{
		ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.4f, 1.0f), "게임 모드 API 호출 가능");
		ImGui::Text("게임 모드: %s", TCHAR_TO_UTF8(*GameMode->GetClass()->GetName()));
	}
	else
	{
		ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "게임 모드 API 호출 불가");
		ImGui::TextWrapped("현재 월드에 초기화된 BornToEndureGameModeBase가 필요합니다. 싱글 플레이 또는 서버에서 테스트하세요.");
	}

	ImGui::Separator();
	EGameDebugAction Action = EGameDebugAction::None;
	ImGui::BeginDisabled(!bCanCall);
	if (ImGui::Button("게임 시작"))
	{
		Action = EGameDebugAction::StartGame;
	}
	ImGui::SameLine();
	if (ImGui::Button("스폰 시작"))
	{
		Action = EGameDebugAction::SpawnWave;
	}
	ImGui::SameLine();
	if (ImGui::Button("게임 종료"))
	{
		Action = EGameDebugAction::EndGame;
	}
	ImGui::EndDisabled();
	ImGui::TextWrapped("게임 시작: StartGame()을 호출하며 스폰도 시작합니다. 스폰 시작: SpawnWave()를 직접 호출합니다.");
	ImGui::TextWrapped("게임 종료: EndGame()을 호출하여 스폰을 중지합니다. 에디터 플레이(PIE)는 종료하지 않습니다.");
	ImGui::TextWrapped("스폰 테스트에는 레벨에 배치된 EnemySpawnDirector와 설정된 EnemySpawner가 필요합니다.");
	ImGui::Separator();
	ImGui::TextWrapped("마지막 요청: %s", TCHAR_TO_UTF8(*LastActionMessage));
	ImGui::TextDisabled("호출 내역입니다. 실제 게임 진행·스폰 상태는 공개 조회 API가 없습니다.");
	return Action;
}

void AGameDebugActor::ExecuteGameAction(EGameDebugAction Action)
{
	UWorld* World = GetWorld();
	if (!IsValid(World) || IsActorBeingDestroyed())
	{
		return;
	}
	ABornToEndureGameModeBase* GameMode = World->GetAuthGameMode<ABornToEndureGameModeBase>();
	if (!IsValid(GameMode) || GameMode->IsActorBeingDestroyed() || !GameMode->HasActorBegunPlay())
	{
		LastActionMessage = TEXT("호출하지 못했습니다. 게임 모드를 사용할 수 없습니다.");
		return;
	}

	// Record the request before calling out; APIs return void and do not report success.
	switch (Action)
	{
	case EGameDebugAction::StartGame:
		LastActionMessage = TEXT("StartGame() 호출 요청을 전달했습니다. 실제 결과는 출력 로그를 확인하세요.");
		GameMode->StartGame();
		break;
	case EGameDebugAction::SpawnWave:
		LastActionMessage = TEXT("SpawnWave() 호출 요청을 전달했습니다. 실제 결과는 출력 로그를 확인하세요.");
		GameMode->SpawnWave();
		break;
	case EGameDebugAction::EndGame:
		LastActionMessage = TEXT("EndGame() 호출 요청을 전달했습니다. 실제 결과는 출력 로그를 확인하세요.");
		GameMode->EndGame();
		break;
	default:
		break;
	}
}
#endif
