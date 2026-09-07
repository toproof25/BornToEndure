#include "Item/PetItemDebugActor.h"

#if !UE_BUILD_SHIPPING
#include "Character/Pet/PetCompanionCharacter.h"
#include "Component/PetItemComponent.h"
#include "Component/PetManagerComponent.h"
#include "Component/PetStatComponent.h"
#include "Data/PetProjectileItemDataAsset.h"
#include "Engine/DataTable.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Data/PetSynergyDataAsset.h"
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
	if (CatalogLoadHandle.IsValid()) CatalogLoadHandle->CancelHandle();
	CatalogLoadHandle.Reset();
	CatalogEntries.Reset();
	SynergyOptions.Reset();
	CatalogAssets.Reset();
#endif
	LoadedWeaponItems.Reset();
	Super::EndPlay(EndPlayReason);
}

void APetItemDebugActor::RenderImGui()
{
#if !UE_BUILD_SHIPPING
	if (!IsValid(GetWorld())) return;
	const FString Title = FString::Printf(TEXT("펫 아이템 테스트###PetItemDebug_%s"), *GetPathName());
	ImGui::SetNextWindowSize(ImVec2(1100, 600), ImGuiCond_FirstUseEver);
	if (ImGui::Begin(TCHAR_TO_UTF8(*Title)))
	{
		ImGui::TextDisabled("대상 선택 → 아이템 검색 → 지급 (클릭당 1개)");
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
	if (!SelectedPet.IsValid() || SelectedPet->IsActorBeingDestroyed() || !Pets.Contains(SelectedPet.Get()))
	{
		SelectedPet.Reset();
		LastResult.Reset();
		for (const auto& Entry : Pets)
		{
			if (IsValid(Entry.Get()) && !Entry->IsActorBeingDestroyed())
			{
				SelectedPet = Entry.Get();
				break;
			}
		}
	}
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

#if !UE_BUILD_SHIPPING
namespace PetItemDebug
{
	FString DescribeModifiers(const TArray<FStatModifier>& Modifiers)
	{
		TArray<FString> Lines;
		for (const FStatModifier& Mod : Modifiers)
		{
			const FString Name = StaticEnum<EPetStatType>()->GetDisplayNameTextByValue(static_cast<int64>(Mod.StatType)).ToString();
			switch (Mod.ModType)
			{
			case EStatModifierType::Additive:
				Lines.Add(FString::Printf(TEXT("%s: 고정 %+.3f"), *Name, Mod.Value)); break;
			case EStatModifierType::Multiplicative:
				Lines.Add(FString::Printf(TEXT("%s: 곱연산 %+.1f%%"), *Name, Mod.Value * 100.f)); break;
			case EStatModifierType::Override:
				Lines.Add(FString::Printf(TEXT("%s: 덮어쓰기 %.3f"), *Name, Mod.Value)); break;
			default: Lines.Add(TEXT("알 수 없는 Modifier 연산")); break;
			}
		}
		return Lines.IsEmpty() ? TEXT("스탯 효과 없음") : FString::Join(Lines, TEXT("\n"));
	}

	// Submit only a fixed-size item. Text is drawn inside its clip rectangle and cannot grow the row.
	void FixedCell(const FString& Text, float Height, const FString& Tooltip = FString())
	{
		const ImVec2 Position = ImGui::GetCursorScreenPos();
		const float Width = FMath::Max(1.f, ImGui::GetContentRegionAvail().x);
		ImGui::Dummy(ImVec2(Width, Height));
		const bool bHovered = ImGui::IsItemHovered();
		const FString SingleLine = Text.Replace(TEXT("\r"), TEXT("")).Replace(TEXT("\n"), TEXT(" / "));
		ImDrawList* DrawList = ImGui::GetWindowDrawList();
		DrawList->PushClipRect(Position, ImVec2(Position.x + Width, Position.y + Height), true);
		DrawList->AddText(ImVec2(Position.x, Position.y + FMath::Max(0.f, (Height - ImGui::GetFontSize()) * 0.5f)),
			ImGui::GetColorU32(ImGuiCol_Text), TCHAR_TO_UTF8(*SingleLine));
		DrawList->PopClipRect();
		if (bHovered)
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 32.f);
			ImGui::TextUnformatted(TCHAR_TO_UTF8(*(Tooltip.IsEmpty() ? Text : Tooltip)));
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}
}

void APetItemDebugActor::RefreshCatalog(UDataTable* Table, EItemType Type)
{
	if (CatalogLoadHandle.IsValid()) CatalogLoadHandle->CancelHandle();
	CatalogLoadHandle.Reset();
	CatalogAssets.Reset();
	CatalogEntries.Reset();
	SynergyOptions.Reset();
	ListedTable = Table;
	ListedType = Type;
	RowNames = Table->GetRowNames();
	RowNames.Sort(FNameLexicalLess());
	TArray<FSoftObjectPath> Paths;
	if (Type == EItemType::Weapon)
	{
		for (FName RowName : RowNames)
		{
			const FWeaponItemDataRow* Row = Table->FindRow<FWeaponItemDataRow>(RowName, TEXT("PetItemDebug"), false);
			if (Row && !Row->WeaponItemDataAsset.IsNull()) Paths.AddUnique(Row->WeaponItemDataAsset.ToSoftObjectPath());
		}
	}
	UAssetManager& Assets = UAssetManager::Get();
	TArray<FPrimaryAssetId> SynergyIds;
	Assets.GetPrimaryAssetIdList(FPrimaryAssetType(TEXT("PetSynergy")), SynergyIds);
	for (const FPrimaryAssetId& Id : SynergyIds)
	{
		const FSoftObjectPath Path = Assets.GetPrimaryAssetPath(Id);
		if (Path.IsValid()) Paths.AddUnique(Path);
	}
	bCatalogLoading = true;
	if (Paths.IsEmpty())
	{
		OnCatalogLoaded();
		return;
	}
	CatalogLoadHandle = Assets.GetStreamableManager().RequestAsyncLoad(Paths,
		FStreamableDelegate::CreateUObject(this, &APetItemDebugActor::OnCatalogLoaded));
	if (!CatalogLoadHandle.IsValid()) OnCatalogLoaded();
}

void APetItemDebugActor::OnCatalogLoaded()
{
	if (CatalogLoadHandle.IsValid())
	{
		TArray<UObject*> Objects;
		CatalogLoadHandle->GetLoadedAssets(Objects);
		for (UObject* Object : Objects) if (IsValid(Object)) CatalogAssets.Add(Object);
	}
	bCatalogLoading = false;
	UDataTable* Table = ListedTable.Get();
	if (!IsValid(Table) || PetItemDebug::GetTable(ResolveItemPool(), ListedType) != Table) return;
	CatalogEntries.Reset();
	SynergyOptions.Reset();
	for (FName RowName : RowNames)
	{
		const FStatItemDataRow* StatRow = ListedType == EItemType::Stat
			? Table->FindRow<FStatItemDataRow>(RowName, TEXT("PetItemDebug"), false) : nullptr;
		const FWeaponItemDataRow* WeaponRow = ListedType == EItemType::Weapon
			? Table->FindRow<FWeaponItemDataRow>(RowName, TEXT("PetItemDebug"), false) : nullptr;
		const FItemDataRow* Row = StatRow ? static_cast<const FItemDataRow*>(StatRow) : WeaponRow;
		if (!Row) continue;
		FCatalogEntry Entry;
		Entry.RowName = RowName;
		Entry.Name = Row->ItemText.Name.IsEmpty() ? RowName.ToString() : Row->ItemText.Name.ToString();
		Entry.ItemId = Row->ItemID.ToString();
		Entry.Description = Row->ItemText.Description.ToString();
		FGameplayTagContainer ItemSynergyTags;
		bool bTagsKnown = true;
		if (StatRow)
		{
			Entry.Effects = PetItemDebug::DescribeModifiers(StatRow->StatModifiers);
			ItemSynergyTags = StatRow->SynergyTags;
		}
		else
		{
			const UPetProjectileItemDataAsset* Asset = Cast<UPetProjectileItemDataAsset>(WeaponRow->WeaponItemDataAsset.Get());
			if (IsValid(Asset))
			{
				ItemSynergyTags = Asset->SynergyTags;
				const FProjectileModifierData& Mod = Asset->ProjectileModifier;
				const FString Pattern = StaticEnum<EProjectilePattern>()->GetDisplayNameTextByValue(static_cast<int64>(Mod.Pattern)).ToString();
				Entry.Effects = FString::Printf(TEXT("발사 수 %+d / 크기 x%.2f / 속도 x%.2f\n패턴: %s\n속성: %s\n교체 발사체: %s"),
					Mod.ProjectileCountAdd, Mod.SizeMultiplier, Mod.SpeedMultiplier, *Pattern,
					*Mod.ElementType.ToString(), Mod.OverrideProjectileClass.IsNull() ? TEXT("없음") : *Mod.OverrideProjectileClass.GetAssetName());
			}
			else
			{
				Entry.Effects = TEXT("무기 에셋 로드 실패 / 타입 불일치");
				bTagsKnown = false;
			}
		}
		TArray<FString> Sets;
		for (const auto& Object : CatalogAssets)
		{
			const UPetSynergyDataAsset* Set = Cast<UPetSynergyDataAsset>(Object.Get());
			if (!IsValid(Set)) continue;
			// Catalog membership only: contributes an exact required tag. No activation calculation.
			bool bRelated = false;
			TArray<FString> Requirements;
			for (const auto& Requirement : Set->RequiredTagCounts)
			{
				bRelated |= ItemSynergyTags.HasTagExact(Requirement.Key);
				Requirements.Add(FString::Printf(TEXT("%s x%d"), *Requirement.Key.ToString(), Requirement.Value));
			}
			if (!bRelated) continue;
			Requirements.Sort();
			const FString Name = Set->SynergyName.IsEmpty() ? Set->GetName() : Set->SynergyName.ToString();
			const FString Key = Set->GetPathName();
			SynergyOptions.Add(Key, FString::Printf(TEXT("세트: %s (%s)"), *Name, *Set->GetName()));
			Entry.SynergyKeys.Add(Key);
			Sets.Add(FString::Printf(TEXT("%s [%s]"), *Name, *FString::Join(Requirements, TEXT(", "))));
		}
		Sets.Sort();
		for (const FGameplayTag& Tag : ItemSynergyTags)
		{
			const FString Key = TEXT("Tag:") + Tag.ToString();
			SynergyOptions.Add(Key, TEXT("태그: ") + Tag.ToString());
			Entry.SynergyKeys.Add(Key);
		}
		Entry.Synergies = !bTagsKnown ? TEXT("확인 불가") : Sets.IsEmpty() ? TEXT("연결된 세트 없음") : FString::Join(Sets, TEXT("\n"));
		if (!ItemSynergyTags.IsEmpty()) Entry.Synergies += TEXT("\n태그: ") + ItemSynergyTags.ToStringSimple();
		CatalogEntries.Add(MoveTemp(Entry));
	}
	if (!SynergyOptions.Contains(SelectedSynergy)) SelectedSynergy.Reset();
}

void APetItemDebugActor::DrawCatalog(UItemPoolSubsystem* Pool, EItemType Type)
{
	UDataTable* Table = PetItemDebug::GetTable(Pool, Type);
	if (!Table)
	{
		ImGui::TextDisabled("ItemPool 테이블이 없거나 행 타입이 맞지 않습니다.");
		return;
	}
	const bool bRefresh = ImGui::SmallButton("목록 새로고침");
	if (ListedTable.Get() != Table || ListedType != Type || bRefresh) RefreshCatalog(Table, Type);
	ImGui::SameLine();
	ImGui::TextDisabled("%s · %d개", TCHAR_TO_UTF8(*Table->GetName()), RowNames.Num());
	if (bCatalogLoading)
	{
		ImGui::TextDisabled("효과 / 시너지 데이터 로딩 중...");
		return;
	}
	const FString Preview = SelectedSynergy.IsEmpty() ? TEXT("전체") : SynergyOptions.FindRef(SelectedSynergy);
	if (ImGui::BeginCombo("시너지별 조회", TCHAR_TO_UTF8(*Preview)))
	{
		if (ImGui::Selectable("전체", SelectedSynergy.IsEmpty())) SelectedSynergy.Reset();
		TArray<FString> Keys;
		SynergyOptions.GetKeys(Keys);
		Keys.Sort();
		for (const FString& Key : Keys)
		{
			ImGui::PushID(TCHAR_TO_UTF8(*Key));
			if (ImGui::Selectable(TCHAR_TO_UTF8(*SynergyOptions.FindChecked(Key)), SelectedSynergy == Key)) SelectedSynergy = Key;
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}
	ImGui::TextDisabled("세트 요구 태그에 관련된 아이템을 조회합니다. 긴 내용은 마우스를 올려 확인하세요.");
	const FString Filter = UTF8_TO_TCHAR(SearchText);
	FName RequestedRow = NAME_None;
	int32 VisibleCount = 0;
	const float RowHeight = FMath::Max(48.f, ImGui::GetFrameHeight() + 16.f);
	const float ContentHeight = RowHeight - 2.f * ImGui::GetStyle().CellPadding.y;
	if (ImGui::BeginTable("CatalogEffects", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH |
		ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable, ImVec2(0, 0)))
	{
		ImGui::TableSetupColumn("아이템 이름", ImGuiTableColumnFlags_WidthStretch, 1.5f);
		ImGui::TableSetupColumn("행 이름", ImGuiTableColumnFlags_WidthStretch, 1.f);
		ImGui::TableSetupColumn("적용 효과", ImGuiTableColumnFlags_WidthStretch, 2.f);
		ImGui::TableSetupColumn("시너지 세트", ImGuiTableColumnFlags_WidthStretch, 2.f);
		ImGui::TableSetupColumn("지급", ImGuiTableColumnFlags_WidthFixed, 70.f);
		ImGui::TableSetupScrollFreeze(0, 1);
		ImGui::TableHeadersRow();
		for (const FCatalogEntry& Entry : CatalogEntries)
		{
			if (!SelectedSynergy.IsEmpty() && !Entry.SynergyKeys.Contains(SelectedSynergy)) continue;
			if (!Filter.IsEmpty() && !Entry.Name.Contains(Filter) && !Entry.RowName.ToString().Contains(Filter)
				&& !Entry.ItemId.Contains(Filter)) continue;
			++VisibleCount;
			ImGui::PushID(TCHAR_TO_UTF8(*Entry.RowName.ToString()));
			ImGui::TableNextRow(ImGuiTableRowFlags_None, RowHeight);
			ImGui::TableSetColumnIndex(0);
			PetItemDebug::FixedCell(Entry.Name, ContentHeight, Entry.Name + TEXT("\n") + Entry.Description + TEXT("\nID: ") + Entry.ItemId);
			ImGui::TableSetColumnIndex(1);
			PetItemDebug::FixedCell(Entry.RowName.ToString(), ContentHeight);
			ImGui::TableSetColumnIndex(2);
			PetItemDebug::FixedCell(Entry.Effects, ContentHeight);
			ImGui::TableSetColumnIndex(3);
			PetItemDebug::FixedCell(Entry.Synergies, ContentHeight);
			ImGui::TableSetColumnIndex(4);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (ContentHeight - ImGui::GetFrameHeight()) * 0.5f);
			APetCompanionCharacter* Pet = SelectedPet.Get();
			const bool bReady = IsValid(Pet) && !Pet->IsActorBeingDestroyed()
				&& IsValid(Pet->GetItemComponent()) && IsValid(Pet->GetStatComponent());
			ImGui::BeginDisabled(!bReady);
			if (ImGui::Button("지급", ImVec2(58.f, ImGui::GetFrameHeight()))) RequestedRow = Entry.RowName;
			ImGui::EndDisabled();
			ImGui::PopID();
		}
		if (VisibleCount == 0)
		{
			ImGui::TableNextRow(ImGuiTableRowFlags_None, RowHeight);
			ImGui::TableSetColumnIndex(0);
			PetItemDebug::FixedCell(TEXT("표시할 아이템 없음"), ContentHeight);
		}
		ImGui::EndTable();
	}
	if (!RequestedRow.IsNone()) GiveItem(Type, RequestedRow);
}
#endif
