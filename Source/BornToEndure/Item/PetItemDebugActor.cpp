#include "Item/PetItemDebugActor.h"

#if !UE_BUILD_SHIPPING
#include "Character/Pet/PetCompanionCharacter.h"
#include "Component/PetItemComponent.h"
#include "Component/PetManagerComponent.h"
#include "Component/PetStatComponent.h"
#include "Data/PetProjectileItemDataAsset.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Subsystem/ItemPoolSubsystem.h"
#include "ImGuiModule.h"
#include "imgui.h"

namespace PetItemDebug
{
	UDataTable* GetTable(UItemPoolSubsystem* Pool, EItemType Type)
	{
		if (!IsValid(Pool)) return nullptr;
		UDataTable* Table = Type == EItemType::Stat ? Pool->StatDataTable : Pool->WeaponDataTable;
		const UScriptStruct* Expected = Type == EItemType::Stat
			? FStatItemDataRow::StaticStruct() : FWeaponItemDataRow::StaticStruct();
		return IsValid(Table) && Table->GetRowStruct() && Table->GetRowStruct()->IsChildOf(Expected)
			? Table : nullptr;
	}
}
#endif

APetItemDebugActor::APetItemDebugActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APetItemDebugActor::BeginPlay()
{
	Super::BeginPlay();
#if !UE_BUILD_SHIPPING
	if (IsValid(GetWorld()) && FImGuiModule::IsAvailable())
	{
		ImGuiDelegateHandle = FImGuiModule::Get().AddWorldImGuiDelegate(
			GetWorld(), FImGuiDelegate::CreateUObject(this, &APetItemDebugActor::RenderImGui));
	}
#endif
}

void APetItemDebugActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
#if !UE_BUILD_SHIPPING
	if (FImGuiModule::IsAvailable() && ImGuiDelegateHandle.IsValid())
	{
		FImGuiModule::Get().RemoveImGuiDelegate(ImGuiDelegateHandle);
	}
	ImGuiDelegateHandle.Reset();
	SelectedPet.Reset();
	ListedTable.Reset();
	RowNames.Reset();
#endif
	LoadedWeaponItems.Reset();
	Super::EndPlay(EndPlayReason);
}

void APetItemDebugActor::RenderImGui()
{
#if !UE_BUILD_SHIPPING
	if (!IsValid(GetWorld())) return;
	const FString Title = FString::Printf(TEXT("펫 아이템 테스트###PetItemDebug_%s"), *GetPathName());
	ImGui::SetNextWindowSize(ImVec2(760, 520), ImGuiCond_FirstUseEver);
	if (ImGui::Begin(TCHAR_TO_UTF8(*Title)))
	{
		ImGui::TextDisabled("대상 선택 → 아이템 검색 → 1개 지급");
		UPetManagerComponent* Manager = ResolvePetManager();
		DrawPetSelection(Manager);
		if (APetCompanionCharacter* Pet = SelectedPet.Get())
		{
			if (IsValid(Pet->GetItemComponent()))
			{
				ImGui::TextDisabled("보유 인스턴스: %d", Pet->GetItemComponent()->GetOwnedItems().Num());
			}
		}
		if (!LastResult.IsEmpty())
		{
			ImGui::TextColored(bLastSucceeded ? ImVec4(0.4f, 0.85f, 0.5f, 1) : ImVec4(1, 0.7f, 0.3f, 1),
				"%s", TCHAR_TO_UTF8(*LastResult));
		}
		ImGui::Separator();
		ImGui::InputTextWithHint("##Search", "이름 / 행 이름 / ID 검색", SearchText, sizeof(SearchText));
		UItemPoolSubsystem* Pool = ResolveItemPool();
		if (!IsValid(Pool))
		{
			ImGui::TextDisabled("ItemPool을 사용할 수 없습니다. GameInstance 초기화를 확인하세요.");
		}
		else if (ImGui::BeginTabBar("ItemTypes"))
		{
			if (ImGui::BeginTabItem("스탯"))
			{
				DrawCatalog(Pool, EItemType::Stat);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("무기"))
			{
				DrawCatalog(Pool, EItemType::Weapon);
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::End();
#endif
}

#if !UE_BUILD_SHIPPING
UPetManagerComponent* APetItemDebugActor::ResolvePetManager() const
{
	UWorld* World = GetWorld();
	APlayerController* Controller = IsValid(World) ? World->GetFirstPlayerController() : nullptr;
	APawn* Pawn = IsValid(Controller) ? Controller->GetPawn() : nullptr;
	return IsValid(Pawn) ? Pawn->FindComponentByClass<UPetManagerComponent>() : nullptr;
}

UItemPoolSubsystem* APetItemDebugActor::ResolveItemPool() const
{
	UWorld* World = GetWorld();
	UGameInstance* Instance = IsValid(World) ? World->GetGameInstance() : nullptr;
	return IsValid(Instance) ? Instance->GetSubsystem<UItemPoolSubsystem>() : nullptr;
}

void APetItemDebugActor::DrawPetSelection(UPetManagerComponent* Manager)
{
	if (!IsValid(Manager))
	{
		SelectedPet.Reset();
		ImGui::TextDisabled("플레이어 / PetManager를 기다리는 중입니다.");
		return;
	}
	const auto& Pets = Manager->GetPetList();
	if (!SelectedPet.IsValid() || !Pets.Contains(SelectedPet.Get())) SelectedPet.Reset();
	const FString Preview = SelectedPet.IsValid() ? SelectedPet->GetName() : TEXT("펫을 선택하세요");
	if (ImGui::BeginCombo("대상", TCHAR_TO_UTF8(*Preview)))
	{
		for (const auto& Entry : Pets)
		{
			APetCompanionCharacter* Pet = Entry.Get();
			if (!IsValid(Pet) || Pet->IsActorBeingDestroyed()) continue;
			ImGui::PushID(Pet);
			if (ImGui::Selectable(TCHAR_TO_UTF8(*Pet->GetName()), SelectedPet.Get() == Pet))
			{
				SelectedPet = Pet;
				LastResult.Reset();
			}
			ImGui::PopID();
		}
		if (Pets.IsEmpty()) ImGui::TextDisabled("소유한 펫이 없습니다.");
		ImGui::EndCombo();
	}
}

void APetItemDebugActor::DrawCatalog(UItemPoolSubsystem* Pool, EItemType Type)
{
	UDataTable* Table = PetItemDebug::GetTable(Pool, Type);
	if (!Table)
	{
		ImGui::TextDisabled("ItemPool 테이블이 없거나 행 타입이 맞지 않습니다.");
		ImGui::TextWrapped("GameInstance의 해당 아이템 테이블 설정을 확인하세요.");
		return;
	}
	const bool bRefresh = ImGui::SmallButton("목록 새로고침");
	if (ListedTable.Get() != Table || bRefresh)
	{
		ListedTable = Table;
		RowNames = Table->GetRowNames();
		RowNames.Sort(FNameLexicalLess());
	}
	ImGui::SameLine();
	ImGui::TextDisabled("%s · %d개", TCHAR_TO_UTF8(*Table->GetName()), RowNames.Num());
	UDataTable* LegacyTable = Type == EItemType::Stat ? StatItemDataTable.Get() : WeaponItemDataTable.Get();
	if (IsValid(LegacyTable) && LegacyTable != Table)
	{
		ImGui::TextDisabled("Actor에 지정된 이전 테이블 대신 실제 ItemPool 테이블을 표시합니다.");
	}
	const FString Filter = UTF8_TO_TCHAR(SearchText);
	FName RequestedRow = NAME_None;
	int32 VisibleCount = 0;
	if (ImGui::BeginTable("Catalog", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH |
		ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable, ImVec2(0, 0)))
	{
		ImGui::TableSetupColumn("아이템", ImGuiTableColumnFlags_WidthStretch, 2);
		ImGui::TableSetupColumn("행 이름", ImGuiTableColumnFlags_WidthStretch, 1);
		ImGui::TableSetupColumn("지급", ImGuiTableColumnFlags_WidthFixed, 85);
		ImGui::TableSetupScrollFreeze(0, 1);
		ImGui::TableHeadersRow();
		for (FName RowName : RowNames)
		{
			const FItemDataRow* Row = Type == EItemType::Stat
				? static_cast<const FItemDataRow*>(Table->FindRow<FStatItemDataRow>(RowName, TEXT("PetItemDebug"), false))
				: static_cast<const FItemDataRow*>(Table->FindRow<FWeaponItemDataRow>(RowName, TEXT("PetItemDebug"), false));
			if (!Row) continue;
			const FString Name = Row->ItemText.Name.ToString();
			if (!Filter.IsEmpty() && !Name.Contains(Filter) && !RowName.ToString().Contains(Filter)
				&& !Row->ItemID.ToString().Contains(Filter)) continue;
			++VisibleCount;
			ImGui::PushID(TCHAR_TO_UTF8(*RowName.ToString()));
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextUnformatted(TCHAR_TO_UTF8(*(Name.IsEmpty() ? RowName.ToString() : Name)));
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 28);
				ImGui::TextUnformatted(TCHAR_TO_UTF8(*Row->ItemText.Description.ToString()));
				ImGui::TextDisabled("ID: %s", TCHAR_TO_UTF8(*Row->ItemID.ToString()));
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}
			ImGui::TableSetColumnIndex(1);
			ImGui::TextDisabled("%s", TCHAR_TO_UTF8(*RowName.ToString()));
			ImGui::TableSetColumnIndex(2);
			APetCompanionCharacter* Pet = SelectedPet.Get();
			const bool bReady = IsValid(Pet) && !Pet->IsActorBeingDestroyed()
				&& IsValid(Pet->GetItemComponent()) && IsValid(Pet->GetStatComponent());
			ImGui::BeginDisabled(!bReady);
			if (ImGui::SmallButton("1개 지급")) RequestedRow = RowName;
			ImGui::EndDisabled();
			ImGui::PopID();
		}
		if (VisibleCount == 0)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextDisabled("표시할 아이템이 없습니다.");
		}
		ImGui::EndTable();
	}
	// Production callbacks can mutate runtime state. Invoke only after row iteration ends.
	if (!RequestedRow.IsNone()) GiveItem(Type, RequestedRow);
}

void APetItemDebugActor::GiveItem(EItemType Type, FName RowName)
{
	bLastSucceeded = false;
	UPetManagerComponent* Manager = ResolvePetManager();
	APetCompanionCharacter* Pet = SelectedPet.Get();
	UItemPoolSubsystem* Pool = ResolveItemPool();
	UDataTable* Table = PetItemDebug::GetTable(Pool, Type);
	if (!IsValid(Manager) || !IsValid(Pet) || Pet->IsActorBeingDestroyed()
		|| !Manager->GetPetList().Contains(Pet) || !IsValid(Pet->GetItemComponent())
		|| !IsValid(Pet->GetStatComponent()) || !Table)
	{
		LastResult = TEXT("지급 취소: 대상 또는 ItemPool을 다시 확인하세요.");
		return;
	}
	if (Type == EItemType::Stat)
	{
		if (!Pool->GetStatItemDataRowByID(RowName))
		{
			LastResult = TEXT("지급 취소: 스탯 행을 찾을 수 없습니다.");
			return;
		}
	}
	else
	{
		const FWeaponItemDataRow* Row = Pool->GetWeaponItemDataRowByID(RowName);
		// Explicit button action only; never load assets in the per-frame catalog.
		UPetProjectileItemDataAsset* Asset = Row
			? Cast<UPetProjectileItemDataAsset>(Row->WeaponItemDataAsset.LoadSynchronous()) : nullptr;
		if (!IsValid(Asset))
		{
			LastResult = TEXT("지급 취소: 무기 에셋이 없거나 타입이 맞지 않습니다.");
			return;
		}
		LoadedWeaponItems.AddUnique(Asset);
	}
	TWeakObjectPtr<UPetItemComponent> ItemComponent = Pet->GetItemComponent();
	const int32 Before = ItemComponent->GetOwnedItems().Num();
	const FString PetName = Pet->GetName();
	FItemDataHandle Handle;
	Handle.ItemType = Type;
	// FindRow resolves table row keys, not the optional ItemID field.
	Handle.ItemRowName = RowName;
	Manager->GiveItemToPet(Pet, Handle);
	bLastSucceeded = ItemComponent.IsValid() && ItemComponent->GetOwnedItems().Num() == Before + 1;
	LastResult = FString::Printf(TEXT("%s · %s → %s"),
		bLastSucceeded ? TEXT("인스턴스 추가 확인") : TEXT("추가 미확인"), *RowName.ToString(), *PetName);
}
#endif
