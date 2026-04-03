// Fill out your copyright notice in the Description page of Project Settings.


#include "TestCode/Test_BTSandBTTask_BP_CPP.h"

#if !UE_BUILD_SHIPPING
	#include "imgui.h"
	#include "ImGuiModule.h"     
	#include "ImGuiDelegates.h" 
#endif

#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Character/Enemy/BaseEnemy.h"

#include "ProfilingDebugging/CpuProfilerTrace.h" // 프로파일링용 헤더 필수


ATest_BTSandBTTask_BP_CPP::ATest_BTSandBTTask_BP_CPP()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ATest_BTSandBTTask_BP_CPP::BeginPlay()
{
	Super::BeginPlay();

	if (FImGuiModule::IsAvailable())
	{
		FImGuiDelegate Delegate = FImGuiDelegate::CreateUObject(this, &ATest_BTSandBTTask_BP_CPP::RenderImGui);
		ImGuiDelegateHandle = FImGuiModule::Get().AddWorldImGuiDelegate(Delegate);
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	LoadEnemy = GetWorld()->SpawnActor<AActor>(EnemyClass, GetActorLocation() + FVector(10000.f, 0.f, 0.f), GetActorRotation(), SpawnParams);
}

void ATest_BTSandBTTask_BP_CPP::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (FImGuiModule::IsAvailable() && ImGuiDelegateHandle.IsValid())
    {
        FImGuiModule::Get().RemoveImGuiDelegate(ImGuiDelegateHandle);
        ImGuiDelegateHandle.Reset();
    }
    Super::EndPlay(EndPlayReason);
}

void ATest_BTSandBTTask_BP_CPP::RenderImGui()
{
	if (ImGui::Begin("BT Service & Task Test"))
	{
		ImGui::Text("Automated Profiling Environment:");
		ImGui::Separator();
		FString ClassName = EnemyClass ? EnemyClass->GetName() : TEXT("None (Set in Details)");
		ImGui::Text("Target: %s", TCHAR_TO_ANSI(*ClassName));

		ImGui::Spacing();

		// 🌟 테스트가 진행 중인지 타이머로 확인하여 중복 실행 방지
		bool bIsTesting = GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(FireTimerHandle);
		if (bIsTesting)
		{
			// 진행 중일 때는 텍스트만 표시하고 버튼을 숨김
			ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Profiling in progress... Please wait 60s.");
		}
		else
		{
			// 대기 중일 때만 버튼 표시
			if (ImGui::Button("Start Auto Trace & Spawn (60s)", ImVec2(-1, 30)))
			{
				OnTestSpawnEnemy();
			}
		}
	}
	ImGui::End();
}

void ATest_BTSandBTTask_BP_CPP::OnTestSpawnEnemy()
{
	if (!EnemyClass || !GetWorld()) return;

	// 🌟 1. 자동 트레이스 시작 및 환경 세팅
	if (GEngine)
	{
		GEngine->Exec(GetWorld(), TEXT("r.VSync 0"));
		GEngine->Exec(GetWorld(), TEXT("t.MaxFPS 120"));

		// 주의: trace.enable 로 채널을 먼저 켜고 시작해야 완벽합니다!
		GEngine->Exec(GetWorld(), TEXT("trace.enable default,cpu,bookmark"));
		GEngine->Exec(GetWorld(), TEXT("trace.start file"));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Trace Started! Generating Data..."));
	}

	// 🌟 2. 북마크 기록
	TRACE_BOOKMARK(TEXT("A/B TEST START: 15 Enemies Spawned"));

	// 3. 적 15마리 스폰 로직
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	FVector BaseLocation = GetActorLocation();
	FRotator BaseRotation = GetActorRotation();

	// 기존 배열이 있다면 비워줍니다 (안전 장치)
	SpawnedEnemies.Empty();

	for (int32 i = 0; i < 100; ++i)
	{
		// Z축(높이) 기준으로 100씩 차이를 두며 스폰 (i가 커질수록 위로 쌓임)
		FVector SpawnLocation = BaseLocation + FVector(0.0f, 0.0f, i * 100.0f);

		AActor* SpawnedEnemy = GetWorld()->SpawnActor<AActor>(EnemyClass, SpawnLocation, BaseRotation, SpawnParams);

		if (SpawnedEnemy)
		{
			SpawnedEnemies.Add(SpawnedEnemy);
		}
	}

	// 4. 타이머 세팅 (정상적으로 스폰된 적이 1마리라도 있다면 60초 타이머 가동)
	if (SpawnedEnemies.Num() > 0)
	{
		float TestDuration = 60.0f;
		GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, this, &ATest_BTSandBTTask_BP_CPP::OnTestEnd, TestDuration, false);
	}
}

void ATest_BTSandBTTask_BP_CPP::OnTestEnd()
{
	// 🌟 1. 종료 북마크 기록
	TRACE_BOOKMARK(TEXT("A/B TEST END: Test Finished"));

	if (GEngine)
	{
		// 🌟 2. 트레이스 자동 종료
		GEngine->Exec(GetWorld(), TEXT("trace.stop"));
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("Trace Stopped! Check your Traces folder."));
	}

	// 🌟 3. 배열을 순회하며 스폰되었던 적 15마리 모두 파괴
	for (AActor* Enemy : SpawnedEnemies)
	{
		if (Enemy && IsValid(Enemy))
		{
			Enemy->Destroy();
		}
	}

	// 파괴 후 배열을 깔끔하게 비워줍니다.
	SpawnedEnemies.Empty();

	// 타이머 핸들 정리
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
	}
}
