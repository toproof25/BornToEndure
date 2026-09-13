#include "DebugCode/PetAddAndRemoveDebugActor.h"

#if !UE_BUILD_SHIPPING
#include "Character/Pet/PetCompanionCharacter.h"
#include "Component/PetManagerComponent.h"
#include "Data/PetBaseDataAsset.h"
#include "Data/DataTableRow/PetDataRow.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Subsystem/ItemPoolSubsystem.h"
#include "ImGuiModule.h"
#include "imgui.h"
#endif

APetAddAndRemoveDebugActor::APetAddAndRemoveDebugActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APetAddAndRemoveDebugActor::BeginPlay()
{
	Super::BeginPlay();
#if !UE_BUILD_SHIPPING
	if (IsValid(GetWorld()) && FImGuiModule::IsAvailable())
	{
		ImGuiDelegateHandle = FImGuiModule::Get().AddWorldImGuiDelegate(
			GetWorld(), FImGuiDelegate::CreateUObject(this, &APetAddAndRemoveDebugActor::RenderImGui));
	}
#endif
}

void APetAddAndRemoveDebugActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
#if !UE_BUILD_SHIPPING
	if (FImGuiModule::IsAvailable() && ImGuiDelegateHandle.IsValid())
	{
		FImGuiModule::Get().RemoveImGuiDelegate(ImGuiDelegateHandle);
	}
	ImGuiDelegateHandle.Reset();
	LastResult.Reset();
#endif
	Super::EndPlay(EndPlayReason);
}

#if !UE_BUILD_SHIPPING
void APetAddAndRemoveDebugActor::RenderImGui()
{
	UWorld* World = GetWorld();
	if (!IsValid(World) || IsActorBeingDestroyed()) return;
	const FString Title = FString::Printf(TEXT("펫 추가 테스트###PetAddDebug_%s"), *GetPathName());
	ImGui::SetNextWindowSize(ImVec2(760.f, 460.f), ImGuiCond_FirstUseEver);
	TSoftObjectPtr<UPetBaseDataAsset> RequestedAsset;
	TWeakObjectPtr<UPetManagerComponent> RequestedManager;
	if (ImGui::Begin(TCHAR_TO_UTF8(*Title)))
	{
		ImGui::TextDisabled("ItemPool PetDataTable 목록 · 추가는 클릭당 1회");
		ImGui::TextDisabled("제거: 플레이어 디버그 창 > 펫 & 동료 > 펫 제거");
		APlayerController* Controller = World->GetFirstPlayerController();
		APawn* Pawn = IsValid(Controller) ? Controller->GetPawn() : nullptr;
		UPetManagerComponent* Manager = IsValid(Pawn) && !Pawn->IsActorBeingDestroyed()
			? Pawn->FindComponentByClass<UPetManagerComponent>() : nullptr;
		const bool bCanAdd = IsValid(Manager) && IsValid(Manager->DefaultPetClass.Get());
		if (IsValid(Manager)) ImGui::Text("대상: %s / 보유: %d", TCHAR_TO_UTF8(*Pawn->GetName()), Manager->GetPetCount());
		if (!bCanAdd) ImGui::TextDisabled("플레이어 PetManager와 DefaultPetClass 설정이 필요합니다.");
		if (!LastResult.IsEmpty()) ImGui::TextWrapped("%s", TCHAR_TO_UTF8(*LastResult));

		UGameInstance* Instance = World->GetGameInstance();
		UItemPoolSubsystem* Pool = IsValid(Instance) ? Instance->GetSubsystem<UItemPoolSubsystem>() : nullptr;
		UDataTable* Table = IsValid(Pool) ? Pool->PetDataTable : nullptr;
		if (!IsValid(Table) || !Table->GetRowStruct() || !Table->GetRowStruct()->IsChildOf(FPetDataRow::StaticStruct()))
		{
			ImGui::TextDisabled("FPetDataRow 형식의 PetDataTable 초기화를 기다리는 중입니다.");
		}
		else if (Table->GetRowMap().IsEmpty()) ImGui::TextDisabled("등록된 펫이 없습니다.");
		else if (ImGui::BeginTable("PetCatalog", 3, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
		{
			ImGui::TableSetupColumn("펫 / 행 이름");
			ImGui::TableSetupColumn("설명");
			ImGui::TableSetupColumn("작업", ImGuiTableColumnFlags_WidthFixed, 80.f);
			ImGui::TableHeadersRow();
			for (const auto& Entry : Table->GetRowMap())
			{
				const FPetDataRow* Row = Table->FindRow<FPetDataRow>(Entry.Key, TEXT("PetAddDebug"), false);
				if (!Row) continue;
				const FString RowName = Entry.Key.ToString();
				ImGui::PushID(TCHAR_TO_UTF8(*RowName));
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::TextUnformatted(TCHAR_TO_UTF8(*Row->Name.ToString()));
				ImGui::TextDisabled("%s", TCHAR_TO_UTF8(*RowName));
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", TCHAR_TO_UTF8(*Row->PetBaseDataAsset.ToString()));
				ImGui::TableSetColumnIndex(1);
				ImGui::TextWrapped("%s", TCHAR_TO_UTF8(*Row->Description.ToString()));
				ImGui::TableSetColumnIndex(2);
				ImGui::BeginDisabled(!bCanAdd || Row->PetBaseDataAsset.IsNull());
				if (ImGui::Button("추가"))
				{
					RequestedAsset = Row->PetBaseDataAsset;
					RequestedManager = Manager;
				}
				ImGui::EndDisabled();
				if (Row->PetBaseDataAsset.IsNull()) ImGui::TextDisabled("에셋 없음");
				ImGui::PopID();
			}
			ImGui::EndTable();
		}
	}
	ImGui::End();

	// Load only on an explicit click, after the table iteration and ImGui stacks end.
	if (!RequestedAsset.IsNull() && RequestedManager.IsValid())
	{
		if (!IsValid(RequestedAsset.LoadSynchronous()))
		{
			LastResult = TEXT("Pet DataAsset 로드 실패: 에셋 경로를 확인하세요.");
			return;
		}
		UPetManagerComponent* Manager = RequestedManager.Get();
		AActor* ManagerOwner = IsValid(Manager) ? Manager->GetOwner() : nullptr;
		if (!IsValid(ManagerOwner) || ManagerOwner->IsActorBeingDestroyed() || !IsValid(Manager->GetWorld())
			|| !IsValid(Manager->DefaultPetClass.Get())) return;
		const int32 PreviousCount = Manager->GetPetCount();
		Manager->SpawnAndAddPet(RequestedAsset);
		// The current production implementation returns nullptr even after adding a pet.
		if (RequestedManager.IsValid() && IsValid(this) && !IsActorBeingDestroyed())
		{
			LastResult = RequestedManager->GetPetCount() > PreviousCount
				? TEXT("보유 펫 수 증가를 확인했습니다.") : TEXT("추가 후 보유 펫 수 증가를 확인하지 못했습니다.");
		}
	}
}
#endif
