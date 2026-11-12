// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugCode/GameDebugActor.h"

#if !UE_BUILD_SHIPPING
#include "Character/Player/PlayerCharacter.h"
#include "Component/PlayerHealthComponent.h"
#include "Core/BornToEndureGameModeBase.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
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
	LastActionMessage = TEXT("대기");
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
	ImGui::SetNextWindowSize(ImVec2(530.0f, 370.0f), ImGuiCond_FirstUseEver);
	if (ImGui::Begin(TCHAR_TO_UTF8(*WindowTitle)))
	{
		if (ImGui::BeginTabBar("GameDebugTabs"))
		{
			if (ImGui::BeginTabItem("게임 진행"))
			{
				RequestedAction = DrawGameFlowTab(World->GetAuthGameMode<ABornToEndureGameModeBase>());
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("플레이어"))
			{
				RequestedAction = DrawPlayerTab();
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
	DrawGameStatus(bCanCall ? GameMode : nullptr);
	ImGui::Separator();
	EGameDebugAction Action = EGameDebugAction::None;
	ImGui::BeginDisabled(!bCanCall);
	if (ImGui::Button("게임 시작", ImVec2(125.0f, 0.0f)))
	{
		Action = EGameDebugAction::StartGame;
	}
	ImGui::SameLine();
	if (ImGui::Button("게임 종료", ImVec2(125.0f, 0.0f)))
	{
		Action = EGameDebugAction::EndGame;
	}
	ImGui::EndDisabled();
	const bool bCanSkip = bCanCall && GameMode->GetGameState() == EGameState::Playing
		&& GetWorld()->GetTimerManager().TimerExists(GameMode->GetWaveTimerHandle());
	ImGui::BeginDisabled(!bCanSkip);
	if (ImGui::Button("다음 웨이브로 건너뛰기", ImVec2(260.0f, 0.0f)))
	{
		Action = EGameDebugAction::SkipToNextWave;
	}
	ImGui::EndDisabled();
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
	{
		ImGui::SetTooltip("게임 진행 중 웨이브 타이머가 있을 때 사용할 수 있습니다.");
	}
	ImGui::TextDisabled("최근 호출: %s", TCHAR_TO_UTF8(*LastActionMessage));
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("호출 기록입니다. 실제 처리 결과는 출력 로그를 확인하세요.");
	}
	return Action;
}

AGameDebugActor::EGameDebugAction AGameDebugActor::DrawPlayerTab()
{
	APlayerController* Controller = GetWorld()->GetFirstPlayerController();
	APlayerCharacter* Player = IsValid(Controller) ? Cast<APlayerCharacter>(Controller->GetPawn()) : nullptr;
	UPlayerHealthComponent* Health = IsValid(Player) && !Player->IsActorBeingDestroyed()
		? Player->GetPlayerHealthComp() : nullptr;
	const bool bAvailable = IsValid(Health) && Health->HasBegunPlay() && Player->HasAuthority();

	if (bAvailable)
	{
		ImGui::Text("%s", TCHAR_TO_UTF8(*Player->GetName()));
		const float MaxHealth = Health->GetMaxHealth();
		const float CurrentHealth = Health->GetCurrentHealth();
		const float Fraction = MaxHealth > 0.0f ? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f) : 0.0f;
		const FString Overlay = FString::Printf(TEXT("체력 %.0f / %.0f"), CurrentHealth, MaxHealth);
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.3f, 0.9f, 0.4f, 1.0f));
		ImGui::ProgressBar(Fraction, ImVec2(-1.0f, 22.0f), TCHAR_TO_UTF8(*Overlay));
		ImGui::PopStyleColor();
	}
	else
	{
		ImGui::TextDisabled("조작 가능한 플레이어 없음 (서버 권한 필요)");
	}

	ImGui::Separator();
	const bool bInvincible = bAvailable && Health->GetIsInvincible();
	if (bAvailable)
	{
		ImGui::TextColored(bInvincible ? ImVec4(0.3f, 0.9f, 0.4f, 1.0f) : ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
			"● 무적: %s", bInvincible ? "켜짐" : "꺼짐");
	}
	else
	{
		ImGui::TextDisabled("무적: 확인 불가");
	}
	EGameDebugAction Action = EGameDebugAction::None;
	ImGui::BeginDisabled(!bAvailable || bInvincible);
	if (ImGui::Button("무적 켜기", ImVec2(125.0f, 0.0f)))
	{
		Action = EGameDebugAction::EnableInvincibility;
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!bAvailable || !bInvincible);
	if (ImGui::Button("무적 끄기", ImVec2(125.0f, 0.0f)))
	{
		Action = EGameDebugAction::DisableInvincibility;
	}
	ImGui::EndDisabled();
	ImGui::TextDisabled("최근 호출: %s", TCHAR_TO_UTF8(*LastActionMessage));
	return Action;
}

void AGameDebugActor::DrawGameStatus(ABornToEndureGameModeBase* GameMode)
{
	if (!GameMode)
	{
		ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "게임 모드 없음");
		ImGui::TextDisabled("BornToEndureGameModeBase가 있는 서버 월드가 필요합니다.");
		return;
	}

	const EGameState State = GameMode->GetGameState();
	const char* StateLabel = "알 수 없음";
	ImVec4 StateColor(0.6f, 0.6f, 0.6f, 1.0f);
	switch (State)
	{
	case EGameState::WatingToStart: StateLabel = "시작 대기"; break;
	case EGameState::Playing: StateLabel = "진행 중"; StateColor = ImVec4(0.3f, 0.9f, 0.4f, 1.0f); break;
	case EGameState::Paused: StateLabel = "일시정지"; StateColor = ImVec4(1.0f, 0.75f, 0.2f, 1.0f); break;
	case EGameState::GameOver: StateLabel = "게임 오버"; StateColor = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); break;
	default: break;
	}
	ImGui::TextColored(StateColor, "● %s", StateLabel);
	ImGui::SameLine(190.0f);
	ImGui::Text("WAVE %02d", GameMode->GetCurrentWave());
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("%s", TCHAR_TO_UTF8(*GameMode->GetPathName()));
	}
	const FWaveDifficultyValues& Difficulty = GameMode->GetCurrentWaveDifficulty();
	ImGui::TextDisabled("배율  체력 x%.2f   골드 x%.2f   경험치 x%.2f",
		Difficulty.HealthMultiplier, Difficulty.GoldMultiplier, Difficulty.ExpMultiplier);

	const double WorldTime = GetWorld()->GetTimeSeconds();
	if (State != EGameState::WatingToStart && GameMode->GetStartPlayTime() <= WorldTime)
	{
		const double Elapsed = WorldTime - GameMode->GetStartPlayTime();
		const int32 Minutes = FMath::FloorToInt(Elapsed / 60.0);
		const int32 Seconds = FMath::FloorToInt(FMath::Fmod(Elapsed, 60.0));
		ImGui::Text("경과  %02d:%02d", Minutes, Seconds);
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("시작 시각 기준. 게임 종료 후에도 증가할 수 있습니다.");
	}
	else
	{
		ImGui::TextDisabled("경과  --:--");
	}

	ImGui::Spacing();
	const FTimerHandle& WaveTimer = GameMode->GetWaveTimerHandle();
	const FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	const bool bExists = TimerManager.TimerExists(WaveTimer);
	const bool bActive = bExists && TimerManager.IsTimerActive(WaveTimer);
	const bool bPaused = bExists && TimerManager.IsTimerPaused(WaveTimer);
	const ImVec4 TimerColor = bActive ? ImVec4(0.3f, 0.9f, 0.4f, 1.0f)
		: bPaused ? ImVec4(1.0f, 0.75f, 0.2f, 1.0f) : ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
	ImGui::TextColored(TimerColor, "● 웨이브 타이머: %s", bActive ? "진행" : bPaused ? "일시정지" : bExists ? "대기" : "꺼짐");
	if (bExists)
	{
		const float Rate = TimerManager.GetTimerRate(WaveTimer);
		const float Remaining = TimerManager.GetTimerRemaining(WaveTimer);
		const float Progress = Rate > 0.0f ? FMath::Clamp(1.0f - Remaining / Rate, 0.0f, 1.0f) : 0.0f;
		const FString Overlay = FString::Printf(TEXT("%.1f / %.1f초"), Remaining, Rate);
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, TimerColor);
		ImGui::ProgressBar(Progress, ImVec2(-1.0f, 22.0f), TCHAR_TO_UTF8(*Overlay));
		ImGui::PopStyleColor();
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("남은 시간 / 타이머 주기. 다음 호출 함수는 조회할 수 없습니다.");
	}
	else
	{
		ImGui::ProgressBar(0.0f, ImVec2(-1.0f, 22.0f), "--");
	}
}

void AGameDebugActor::ExecuteGameAction(EGameDebugAction Action)
{
	UWorld* World = GetWorld();
	if (!IsValid(World) || IsActorBeingDestroyed())
	{
		return;
	}
	if (Action == EGameDebugAction::EnableInvincibility || Action == EGameDebugAction::DisableInvincibility)
	{
		APlayerController* Controller = World->GetFirstPlayerController();
		APlayerCharacter* Player = IsValid(Controller) ? Cast<APlayerCharacter>(Controller->GetPawn()) : nullptr;
		UPlayerHealthComponent* Health = IsValid(Player) && !Player->IsActorBeingDestroyed()
			? Player->GetPlayerHealthComp() : nullptr;
		if (IsValid(Health) && Health->HasBegunPlay() && Player->HasAuthority())
		{
			const bool bEnable = Action == EGameDebugAction::EnableInvincibility;
			Health->SetInvincible(bEnable);
			LastActionMessage = bEnable ? TEXT("SetInvincible(true)") : TEXT("SetInvincible(false)");
		}
		else
		{
			LastActionMessage = TEXT("실패 · 플레이어 없음");
		}
		return;
	}
	ABornToEndureGameModeBase* GameMode = World->GetAuthGameMode<ABornToEndureGameModeBase>();
	if (!IsValid(GameMode) || GameMode->IsActorBeingDestroyed() || !GameMode->HasActorBegunPlay())
	{
		LastActionMessage = TEXT("실패 · 게임 모드 없음");
		return;
	}

	// Record the request before calling out; APIs return void and do not report success.
	switch (Action)
	{
	case EGameDebugAction::StartGame:
		LastActionMessage = TEXT("StartGame()");
		GameMode->StartGame();
		break;
	case EGameDebugAction::EndGame:
		LastActionMessage = TEXT("EndGame()");
		GameMode->EndGame();
		break;
	case EGameDebugAction::SkipToNextWave:
		if (GameMode->GetGameState() == EGameState::Playing
			&& World->GetTimerManager().TimerExists(GameMode->GetWaveTimerHandle()))
		{
			const int32 PreviousWave = GameMode->GetCurrentWave();
			const bool bSkipped = GameMode->SkipToNextWave();
			const int32 NewWave = IsValid(GameMode) ? GameMode->GetCurrentWave() : PreviousWave;
			const TCHAR* Result = !bSkipped ? TEXT("실패")
				: NewWave > PreviousWave ? TEXT("반영") : TEXT("값 미변경");
			LastActionMessage = FString::Printf(TEXT("스킵 %d → %d (%s)"),
				PreviousWave, NewWave, Result);
		}
		else
		{
			LastActionMessage = TEXT("건너뛰기 불가");
		}
		break;
	default:
		break;
	}
}
#endif
