#include "TestCode/TestNiagaraPoolActor.h"

#include "Data/DebugFont/KoreanFont.h"
#include "imgui.h"
#include "ImGuiModule.h"     
#include "ImGuiDelegates.h" 

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Data/NiagaraDataAsset.h"

#include "Engine/AssetManager.h"
#include "Subsystem/EffectSubsystem.h"
#include "ProfilingDebugging/CpuProfilerTrace.h" // 프로파일링용 헤더 필수

ATestNiagaraPoolActor::ATestNiagaraPoolActor()
{
	PrimaryActorTick.bCanEverTick = false;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	RootComponent = StaticMesh;
}

void ATestNiagaraPoolActor::BeginPlay()
{
	Super::BeginPlay();

	//LoadKoreanFontToImGui();

	if (FImGuiModule::IsAvailable())
	{
		FImGuiDelegate Delegate = FImGuiDelegate::CreateUObject(this, &ATestNiagaraPoolActor::RenderImGui);
		ImGuiDelegateHandle = FImGuiModule::Get().AddWorldImGuiDelegate(Delegate);
	}
}

void ATestNiagaraPoolActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (FImGuiModule::IsAvailable() && ImGuiDelegateHandle.IsValid())
	{
		FImGuiModule::Get().RemoveImGuiDelegate(ImGuiDelegateHandle);
		ImGuiDelegateHandle.Reset();
	}
	Super::EndPlay(EndPlayReason);
}

void ATestNiagaraPoolActor::RenderImGui()
{
	if (ImGui::Begin("Niagara Pool Test (Strict Profiling)"))
	{
		ImGui::Text("Asset Name: %s", *NiagaraDataAssetName.ToString());
		ImGui::InputInt("Spawn Count", &SpawnCount);

		ImGui::Separator();
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Step 1: Load Asset");
		if (ImGui::Button("Load Asset to Memory"))
		{
			LoadAssetToMemory();
		}

		ImGui::Separator();
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Step 2: Warm-up (Only for Pool)");
		if (ImGui::Button("Warm-up Pool (Spawn & Wait)"))
		{
			WarmUpPool();
		}

		ImGui::Separator();
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Step 3: PROFILING TESTS");

		if (ImGui::Button("TEST: No Pooling (ENCPoolMethod::None)"))
		{
			TestNoPooling();
		}

		if (ImGui::Button("TEST: With Pooling (ENCPoolMethod::AutoRelease)"))
		{
			TestWithPooling();
		}

		ImGui::Separator();
		ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Step 4: CONTINUOUS FIRE (Stress Test)");

		// N초, M개 설정 UI 추가
		ImGui::InputInt("Duration (Seconds)", &FireDurationSeconds);
		ImGui::InputInt("Spawn Count per 0.1s", &FireSpawnCountPerTick);

		// 최소값 방어 로직 (0 이하로 내려가지 않게)
		if (FireDurationSeconds < 1) FireDurationSeconds = 1;
		if (FireSpawnCountPerTick < 1) FireSpawnCountPerTick = 1;

		if (ImGui::Button("Auto Fire (No Pool)"))
		{
			StartContinuousFire(ENCPoolMethod::None);
		}

		ImGui::SameLine(); // 버튼을 가로로 나란히 배치

		if (ImGui::Button("Auto Fire (With Pool)"))
		{
			StartContinuousFire(ENCPoolMethod::AutoRelease);
		}
	}
	ImGui::End();
}

void ATestNiagaraPoolActor::LoadKoreanFontToImGui()
{
	TSharedPtr<ImFontConfig> KoreanFontConfig = MakeShareable(new ImFontConfig());
	if (KoreanFontConfig.IsValid())
	{
		static const ImWchar* KoreanRanges = ImGui::GetIO().Fonts->GetGlyphRangesKorean();
		KoreanFontConfig->FontDataOwnedByAtlas = false;
		KoreanFontConfig->FontData = (void*)KoreanFont_data;
		KoreanFontConfig->FontDataSize = KoreanFont_size;
		KoreanFontConfig->SizePixels = 18.0f;
		KoreanFontConfig->MergeMode = true;
		KoreanFontConfig->GlyphRanges = KoreanRanges;
		KoreanFontConfig->PixelSnapH = true;

		FImGuiModule::Get().GetProperties().AddCustomFont("KoreanFont", KoreanFontConfig);
		FImGuiModule::Get().RebuildFontAtlas();
	}
}

// -------------------------------------------------------------------------
// Helper: 안전하게 로드된 에셋 가져오기
// -------------------------------------------------------------------------
UNiagaraDataAsset* ATestNiagaraPoolActor::GetLoadedNiagaraData()
{
	FPrimaryAssetType AssetType(FName(TEXT("NiagaraDataAsset")));
	FPrimaryAssetId AssetId(AssetType, NiagaraDataAssetName.PrimaryAssetName);

	UAssetManager& AssetManager = UAssetManager::Get();
	return Cast<UNiagaraDataAsset>(AssetManager.GetPrimaryAssetObject(AssetId));
}

// -------------------------------------------------------------------------
// Step 1: 에셋을 미리 메모리에 올려두기 (측정 중 로딩 렉 방지)
// -------------------------------------------------------------------------
void ATestNiagaraPoolActor::LoadAssetToMemory()
{
	FPrimaryAssetType AssetType(FName(TEXT("NiagaraDataAsset")));
	FPrimaryAssetId AssetId(AssetType, NiagaraDataAssetName.PrimaryAssetName);

	UAssetManager::Get().LoadPrimaryAsset(AssetId);
	UE_LOG(LogTemp, Warning, TEXT("Asset Loading Requested. Please wait 1-2 seconds."));
}

// -------------------------------------------------------------------------
// Step 2: 웜업 (미리 생성해서 풀에 반납시키기)
// -------------------------------------------------------------------------
void ATestNiagaraPoolActor::WarmUpPool()
{
	UNiagaraDataAsset* LoadedData = GetLoadedNiagaraData();
	if (!LoadedData || !LoadedData->Niagara)
	{
		UE_LOG(LogTemp, Error, TEXT("Asset not loaded! Click 'Load Asset' first."));
		return;
	}

	// 타이머 없이 그냥 스폰만 해서 풀에 채워 넣습니다.
	for (int32 i = 0; i < SpawnCount; ++i)
	{
		FVector SpawnLoc = GetActorLocation() + FVector(i * 100, 0, 0);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), LoadedData->Niagara, SpawnLoc, FRotator::ZeroRotator, FVector(1.f),
			true, true, ENCPoolMethod::AutoRelease, true
		);
	}
	UE_LOG(LogTemp, Warning, TEXT("Warm-up complete. Wait until particles disappear, then run 'With Pool' test."));
}

// -------------------------------------------------------------------------
// Step 3-A: No Pooling 테스트 (순수 스폰)
// -------------------------------------------------------------------------
void ATestNiagaraPoolActor::TestNoPooling()
{
	UNiagaraDataAsset* LoadedData = GetLoadedNiagaraData();
	if (!LoadedData || !LoadedData->Niagara)
	{
		UE_LOG(LogTemp, Error, TEXT("Asset not loaded! Click 'Load Asset' first."));
		return;
	}

	TRACE_BOOKMARK(TEXT("--- TEST START: NO POOL ---"));

	{
		// 괄호 { } 로 스코프를 묶어서 정확히 For 루프의 시간만 측정합니다.
		TRACE_CPUPROFILER_EVENT_SCOPE(TEST_NoPool_SpawnTime);

		for (int32 i = 0; i < SpawnCount; ++i)
		{
			FVector SpawnLoc = GetActorLocation() + FVector(i * 100, 0, 0);
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(), LoadedData->Niagara, SpawnLoc, FRotator::ZeroRotator, FVector(1.f),
				true, true, ENCPoolMethod::None, true
			);
		}
	}
}

// -------------------------------------------------------------------------
// Step 3-B: With Pooling 테스트 (창고에서 꺼내기)
// -------------------------------------------------------------------------
void ATestNiagaraPoolActor::TestWithPooling()
{
	UNiagaraDataAsset* LoadedData = GetLoadedNiagaraData();
	if (!LoadedData || !LoadedData->Niagara)
	{
		UE_LOG(LogTemp, Error, TEXT("Asset not loaded! Click 'Load Asset' first."));
		return;
	}

	TRACE_BOOKMARK(TEXT("--- TEST START: WITH POOL ---"));

	{
		// 괄호 { } 로 스코프를 묶어서 정확히 For 루프의 시간만 측정합니다.
		TRACE_CPUPROFILER_EVENT_SCOPE(TEST_WithPool_SpawnTime);

		for (int32 i = 0; i < SpawnCount; ++i)
		{
			FVector SpawnLoc = GetActorLocation() + FVector(i * 100, 0, 0);
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(), LoadedData->Niagara, SpawnLoc, FRotator::ZeroRotator, FVector(1.f),
				true, true, ENCPoolMethod::AutoRelease, true
			);
		}
	}
}
// -------------------------------------------------------------------------
// Step 4: 지속 부하 테스트 로직 (N초 동안 0.1초 간격으로 M개씩 스폰)
// -------------------------------------------------------------------------
void ATestNiagaraPoolActor::StartContinuousFire(ENCPoolMethod Method)
{
	// 혹시 이미 타이머가 돌고 있다면 끄기
	GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);

	CurrentTestPoolMethod = Method;
	CurrentFireTick = 0;

	// 0.1초마다 쏘니까, N초 동안 쏘려면 (N * 10)번 실행해야 함
	MaxFireTick = FireDurationSeconds * 10;

	// 1초에 쏘는 총알 수 계산 (UI 피드백용)
	int32 TotalPerSecond = FireSpawnCountPerTick * 10;
	UE_LOG(LogTemp, Warning, TEXT("기관총 연사 시작! (초당 %d개씩 %d초간 스폰)"), TotalPerSecond, FireDurationSeconds);

	// 🌟 [추가] 인사이트 타임라인에 시작 북마크 긋기
	if (Method == ENCPoolMethod::None)
	{
		TRACE_BOOKMARK(TEXT("=== CONTINUOUS FIRE START : NO POOL ==="));
	}
	else
	{
		TRACE_BOOKMARK(TEXT("=== CONTINUOUS FIRE START : WITH POOL ==="));
	}

	// 0.1초 간격으로 무한 반복 타이머 시작
	GetWorld()->GetTimerManager().SetTimer(
		FireTimerHandle,
		this,
		&ATestNiagaraPoolActor::OnContinuousFireTick,
		0.1f,
		true
	);
}

void ATestNiagaraPoolActor::OnContinuousFireTick()
{
	UNiagaraDataAsset* LoadedData = GetLoadedNiagaraData();
	if (!LoadedData || !LoadedData->Niagara)
	{
		GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
		return;
	}

	// 🌟 [변경] 풀링 여부에 따라 영수증(스코프) 이름을 다르게 발급하여 완전히 분리합니다.
	if (CurrentTestPoolMethod == ENCPoolMethod::None)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(ContinuousFire_Tick_NoPool);
		for (int32 i = 0; i < FireSpawnCountPerTick; ++i)
		{
			FVector SpawnLoc = GetActorLocation() + FVector(FMath::RandRange(-200, 200), FMath::RandRange(-200, 200), 0);
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(), LoadedData->Niagara, SpawnLoc, FRotator::ZeroRotator, FVector(1.f),
				true, true, CurrentTestPoolMethod, true
			);
		}
	}
	else
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(ContinuousFire_Tick_WithPool);
		for (int32 i = 0; i < FireSpawnCountPerTick; ++i)
		{
			FVector SpawnLoc = GetActorLocation() + FVector(FMath::RandRange(-200, 200), FMath::RandRange(-200, 200), 0);
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(), LoadedData->Niagara, SpawnLoc, FRotator::ZeroRotator, FVector(1.f),
				true, true, CurrentTestPoolMethod, true
			);
		}
	}

	CurrentFireTick++;

	// 목표한 틱(시간)에 도달하면 타이머 종료
	if (CurrentFireTick >= MaxFireTick)
	{
		GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);

		// 🌟 [추가] 인사이트 타임라인에 종료 북마크 긋기
		if (CurrentTestPoolMethod == ENCPoolMethod::None)
		{
			TRACE_BOOKMARK(TEXT("=== CONTINUOUS FIRE END : NO POOL ==="));
		}
		else
		{
			TRACE_BOOKMARK(TEXT("=== CONTINUOUS FIRE END : WITH POOL ==="));
		}

		UE_LOG(LogTemp, Warning, TEXT("연사 테스트 완료! (총 %d개 스폰됨)"), MaxFireTick * FireSpawnCountPerTick);
	}
}