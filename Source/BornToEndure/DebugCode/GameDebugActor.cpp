// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugCode/GameDebugActor.h"

#if !UE_BUILD_SHIPPING
#include "Core/BornToEndureGameModeBase.h"
#include "Engine/World.h"
#include "ImGuiModule.h"
#include "TimerManager.h"
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
	ImGui::SetNextWindowSize(ImVec2(670.0f, 460.0f), ImGuiCond_FirstUseEver);
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
	ImGui::Text("월드 경과 시간: %.1f초", GetWorld()->GetTimeSeconds());
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
	if (ImGui::Button("웨이브 시작"))
	{
		Action = EGameDebugAction::StartWave;
	}
	ImGui::SameLine();
	if (ImGui::Button("게임 종료"))
	{
		Action = EGameDebugAction::EndGame;
	}
	ImGui::EndDisabled();
	ImGui::TextWrapped("게임 시작: StartGame()이 웨이브 시작 타이머를 등록합니다. 웨이브 시작: StartWave()를 직접 호출합니다.");
	ImGui::TextWrapped("게임 종료: EndGame()이 웨이브 타이머를 해제하고 스폰을 중지합니다. 에디터 플레이(PIE)는 종료하지 않습니다.");
	ImGui::TextWrapped("스폰 테스트에는 레벨에 배치된 EnemySpawnDirector와 설정된 EnemySpawner가 필요합니다.");
	ImGui::Separator();
	if (ImGui::CollapsingHeader("게임 및 타이머 상태", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (bCanCall)
		{
			const EGameState State = GameMode->GetGameState();
			const char* StateLabel = "알 수 없음";
			switch (State)
			{
			case EGameState::WatingToStart: StateLabel = "시작 대기"; break;
			case EGameState::Playing: StateLabel = "플레이 중"; break;
			case EGameState::Paused: StateLabel = "일시정지"; break;
			case EGameState::GameOver: StateLabel = "게임 오버"; break;
			default: break;
			}

			ImGui::Text("게임 상태: %s", StateLabel);
			ImGui::Text("현재 웨이브: %d", GameMode->GetCurrentWave());
			if (State != EGameState::WatingToStart && GameMode->GetStartPlayTime() <= GetWorld()->GetTimeSeconds())
			{
				const double Elapsed = GetWorld()->GetTimeSeconds() - GameMode->GetStartPlayTime();
				ImGui::Text("게임 시작 후 경과: %.1f초", Elapsed);
				ImGui::TextDisabled("시작 시각 기준이며, 종료 후에도 증가할 수 있습니다.");
			}
			else
			{
				ImGui::TextDisabled("게임 시작 후 경과: 시작 전 / 확인 불가");
			}

			ImGui::Separator();
			const FTimerHandle& WaveTimer = GameMode->GetWaveTimerHandle();
			const FTimerManager& TimerManager = GetWorld()->GetTimerManager();
			if (TimerManager.IsTimerActive(WaveTimer))
			{
				ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.4f, 1.0f), "웨이브 타이머: 실행 중");
			}
			else if (TimerManager.IsTimerPaused(WaveTimer))
			{
				ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "웨이브 타이머: 일시정지");
			}
			else if (TimerManager.TimerExists(WaveTimer))
			{
				ImGui::TextDisabled("웨이브 타이머: 등록됨 (실행 대기)");
			}
			else
			{
				ImGui::TextDisabled("웨이브 타이머: 등록되지 않음");
			}
			if (TimerManager.TimerExists(WaveTimer))
			{
				ImGui::Text("남은 시간: %.1f초", TimerManager.GetTimerRemaining(WaveTimer));
				ImGui::Text("타이머 주기: %.1f초", TimerManager.GetTimerRate(WaveTimer));
			}
			ImGui::TextDisabled("다음 타이머 동작은 공개 API로 확인할 수 없습니다.");
		}
		else
		{
			ImGui::TextDisabled("게임 모드 상태를 조회할 수 없습니다.");
		}
		ImGui::TextDisabled("스포너별 타이머 상태 / 남은 시간: 조회 API 없음");
	}
	ImGui::Separator();
	ImGui::TextWrapped("마지막 요청: %s", TCHAR_TO_UTF8(*LastActionMessage));
	ImGui::TextDisabled("마지막 요청은 호출 내역이며 실제 게임·스폰 상태를 뜻하지 않습니다.");
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
	case EGameDebugAction::StartWave:
		LastActionMessage = TEXT("StartWave() 호출 요청을 전달했습니다. 실제 결과는 출력 로그를 확인하세요.");
		GameMode->StartWave();
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
