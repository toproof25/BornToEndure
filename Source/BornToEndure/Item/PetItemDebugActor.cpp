#include "Item/PetItemDebugActor.h"

#include "Component/PetManagerComponent.h"
#include "Character/Pet/PetCompanionCharacter.h"
#include "Character/Player/PlayerCharacter.h"
#include "Stat/PetStatTypes.h"
#include "Engine/AssetManager.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "Data/DataTableRow/ItemDataRow.h"
#include "Data/DataTableRow/StatItemDataRow.h"
#include "Data/DataTableRow/WeaponItemDataRow.h"

#if !UE_BUILD_SHIPPING
#include "imgui.h"
#include "ImGuiModule.h"
#include "ImGuiTextureHandle.h"
#endif


APetItemDebugActor::APetItemDebugActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APetItemDebugActor::BeginPlay()
{
	Super::BeginPlay();

#if !UE_BUILD_SHIPPING
	if (FImGuiModule::IsAvailable())
	{
		FImGuiDelegate Delegate =
			FImGuiDelegate::CreateUObject(this, &APetItemDebugActor::RenderImGui);

		ImGuiDelegateHandle =
			FImGuiModule::Get().AddWorldImGuiDelegate(Delegate);
	}
#endif

	// LoadAllPetItemsAsync();

	// ============================================================
	// StatItem DataTable 테스트 및 초기화
	// ============================================================

	if (StatItemDataTable)
	{
		TArray<FName> RowNames = StatItemDataTable->GetRowNames();

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[PetItemDebug] --- StatItem 데이터 테이블 내부 행 목록 시작 (총 %d개) ---"),
			RowNames.Num()
		);

		for (const FName& Name : RowNames)
		{
			UE_LOG(
				LogTemp,
				Log,
				TEXT("[PetItemDebug][StatItem] 행 이름 발견: [%s]"),
				*Name.ToString()
			);
		}

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[PetItemDebug] --- StatItem 데이터 테이블 내부 행 목록 끝 ---")
		);

		StatItemDataTable->GetAllRows<FStatItemDataRow>(
			TEXT("Debug Actor StatItem DataTable"),
			AllStatItemDataRows
		);
	}
	else
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"[PetItemDebug] StatItemDataTable이 null입니다! "
				"에디터에서 에셋이 할당되었는지 확인하세요."
			)
		);
	}

	// ============================================================
	// WeaponItem DataTable 테스트 및 초기화
	// ============================================================

	if (WeaponItemDataTable)
	{
		TArray<FName> RowNames = WeaponItemDataTable->GetRowNames();

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[PetItemDebug] --- WeaponItem 데이터 테이블 내부 행 목록 시작 (총 %d개) ---"),
			RowNames.Num()
		);

		for (const FName& Name : RowNames)
		{
			UE_LOG(
				LogTemp,
				Log,
				TEXT("[PetItemDebug][WeaponItem] 행 이름 발견: [%s]"),
				*Name.ToString()
			);
		}

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[PetItemDebug] --- WeaponItem 데이터 테이블 내부 행 목록 끝 ---")
		);

		WeaponItemDataTable->GetAllRows<FWeaponItemDataRow>(
			TEXT("Debug Actor WeaponItem DataTable"),
			AllWeaponItemDataRows
		);
	}
	else
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"[PetItemDebug] WeaponItemDataTable이 null입니다! "
				"에디터에서 에셋이 할당되었는지 확인하세요."
			)
		);
	}

	bIsLoadingAssets = false;

	UE_LOG(
		LogTemp,
		Log,
		TEXT(
			"[PetItemDebug] DataTable 초기화 완료 - StatItem: %d개, WeaponItem: %d개"
		),
		AllStatItemDataRows.Num(),
		AllWeaponItemDataRows.Num()
	);
}

void APetItemDebugActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
#if !UE_BUILD_SHIPPING
	if (FImGuiModule::IsAvailable() && ImGuiDelegateHandle.IsValid())
	{
		FImGuiModule::Get().RemoveImGuiDelegate(ImGuiDelegateHandle);
		ImGuiDelegateHandle.Reset();
	}
#endif

	Super::EndPlay(EndPlayReason);
}

void APetItemDebugActor::LoadAllPetItemsAsync()
{
	UAssetManager& AM = UAssetManager::Get();

	TArray<FPrimaryAssetId> ItemIds;
	AM.GetPrimaryAssetIdList(FPrimaryAssetType("PetItem"), ItemIds);

	if (ItemIds.IsEmpty())
	{
		bIsLoadingAssets = false;

		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"[PetItemDebug] 발견된 펫 아이템 에셋이 없습니다. "
				"DefaultGame.ini를 확인하세요."
			)
		);

		return;
	}

	AM.LoadPrimaryAssets(
		ItemIds,
		TArray<FName>(),
		FStreamableDelegate::CreateUObject(
			this,
			&APetItemDebugActor::OnItemsLoaded,
			ItemIds
		)
	);
}

void APetItemDebugActor::OnItemsLoaded(TArray<FPrimaryAssetId> LoadedIds)
{
	UAssetManager& AM = UAssetManager::Get();

	LoadedStatItems.Empty();
	LoadedWeaponItems.Empty();

	for (const FPrimaryAssetId& Id : LoadedIds)
	{
		UObject* LoadedObj = AM.GetPrimaryAssetObject(Id);

		if (UPetStatItemDataAsset* StatItem =
			Cast<UPetStatItemDataAsset>(LoadedObj))
		{
			LoadedStatItems.Add(StatItem);
		}
		else if (UPetProjectileItemDataAsset* WeaponItem =
			Cast<UPetProjectileItemDataAsset>(LoadedObj))
		{
			LoadedWeaponItems.Add(WeaponItem);
		}
	}

	bIsLoadingAssets = false;

	UE_LOG(
		LogTemp,
		Log,
		TEXT(
			"[PetItemDebug] 아이템 로드 완료! StatItem: %d개, WeaponItem: %d개"
		),
		LoadedStatItems.Num(),
		LoadedWeaponItems.Num()
	);
}


#if !UE_BUILD_SHIPPING

void APetItemDebugActor::RenderImGui()
{
	// 1. 미니 컨트롤 패널 (최소화 상태)
	if (!bIsWindowOpen)
	{
		ImGui::SetNextWindowBgAlpha(0.8f);

		if (ImGui::Begin(
			"Pet Debug Tools",
			nullptr,
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_NoSavedSettings))
		{
			if (ImGui::Button(
				"펫 아이템 툴 열기 (Open Debugger)",
				ImVec2(250, 40)))
			{
				bIsWindowOpen = true;
			}
		}

		ImGui::End();
		return;
	}

	// 2. 메인 디버거 윈도우
	ImGui::SetNextWindowSize(
		ImVec2(1000, 600),
		ImGuiCond_FirstUseEver
	);

	if (ImGui::Begin(
		"펫 아이템 실시간 관리자 (Pet Item Debugger)",
		&bIsWindowOpen,
		ImGuiWindowFlags_None))
	{
		APlayerCharacter* PlayerChar =
			Cast<APlayerCharacter>(
				GetWorld()->GetFirstPlayerController()->GetPawn()
			);

		UPetManagerComponent* PetManager =
			PlayerChar
			? PlayerChar->FindComponentByClass<UPetManagerComponent>()
			: nullptr;

		if (!PetManager)
		{
			ImGui::TextColored(
				ImVec4(1.0f, 0.3f, 0.3f, 1.0f),
				"[오류] 플레이어 캐릭터 또는 PetManagerComponent를 찾을 수 없습니다."
			);

			ImGui::End();
			return;
		}

		// 상단 컨트롤
		DrawPetSelectionCombo(PetManager);

		ImGui::Separator();
		ImGui::Spacing();

		// 탭 바 구성
		if (ImGui::BeginTabBar("ItemTypeTabs"))
		{
			if (ImGui::BeginTabItem("스탯 아이템 (Stat Items)"))
			{
				DrawStatItemsTab();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("무기 아이템 (Weapon Items)"))
			{
				DrawWeaponItemsTab();
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}

	ImGui::End();
}

void APetItemDebugActor::DrawPetSelectionCombo(
	UPetManagerComponent* PetManager)
{
	const TArray<TObjectPtr<APetCompanionCharacter>>& OwnedPets =
		PetManager->GetPetList();

	ImGui::TextColored(
		ImVec4(0.7f, 0.9f, 1.0f, 1.0f),
		"타겟 펫 선택 (Target Pet) :"
	);

	ImGui::SameLine();

	APetCompanionCharacter* CurrentPet = SelectedPet.Get();

	if (CurrentPet && !OwnedPets.Contains(CurrentPet))
	{
		CurrentPet = nullptr;
		SelectedPet.Reset();
	}

	FString PreviewName =
		CurrentPet
		? CurrentPet->GetName()
		: TEXT("--- 펫을 선택하세요 ---");

	ImGui::PushItemWidth(350.0f);

	if (ImGui::BeginCombo(
		"##TargetPetCombo",
		TCHAR_TO_UTF8(*PreviewName)))
	{
		if (OwnedPets.IsEmpty())
		{
			ImGui::Selectable(
				"소유 중인 펫이 없습니다.",
				false,
				ImGuiSelectableFlags_Disabled
			);
		}
		else
		{
			for (int32 i = 0; i < OwnedPets.Num(); ++i)
			{
				APetCompanionCharacter* Pet = OwnedPets[i].Get();

				if (!Pet)
				{
					continue;
				}

				bool bIsSelected = CurrentPet == Pet;

				FString DisplayName =
					FString::Printf(
						TEXT("[%d] %s"),
						i,
						*Pet->GetName()
					);

				if (ImGui::Selectable(
					TCHAR_TO_UTF8(*DisplayName),
					bIsSelected))
				{
					SelectedPet = Pet;
				}

				if (bIsSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
		}

		ImGui::EndCombo();
	}

	ImGui::PopItemWidth();
}

void APetItemDebugActor::DrawStatItemsTab()
{
	if (AllStatItemDataRows.IsEmpty())
	{
		ImGui::TextDisabled("로드된 스탯 아이템이 없습니다.");
		return;
	}

	const ImGuiTableFlags TableFlags =
		ImGuiTableFlags_Borders |
		ImGuiTableFlags_RowBg |
		ImGuiTableFlags_Resizable |
		ImGuiTableFlags_SizingStretchProp |
		ImGuiTableFlags_PadOuterX;

	if (ImGui::BeginTable("StatItemsTable", 5, TableFlags))
	{
		ImGui::TableSetupColumn(
			"아이템 ID",
			ImGuiTableColumnFlags_WidthStretch,
			1.0f
		);

		ImGui::TableSetupColumn(
			"Item Name",
			ImGuiTableColumnFlags_WidthStretch,
			2.0f
		);

		ImGui::TableSetupColumn(
			"Item Description",
			ImGuiTableColumnFlags_WidthStretch,
			1.0f
		);

		ImGui::TableSetupColumn(
			"Item Type",
			ImGuiTableColumnFlags_WidthStretch,
			3.0f
		);

		ImGui::TableSetupColumn(
			"액션",
			ImGuiTableColumnFlags_WidthFixed,
			100.0f
		);

		ImGui::TableHeadersRow();

		for (FStatItemDataRow* DataRow : AllStatItemDataRows)
		{
			if (!DataRow)
			{
				continue;
			}

			ImGui::PushID(DataRow);
			ImGui::TableNextRow();

			// ----------------------------------------------------
			// 1. 아이템 ID
			// ----------------------------------------------------

			ImGui::TableSetColumnIndex(0);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);

			ImGui::TextColored(
				ImVec4(0.5f, 1.0f, 0.5f, 1.0f),
				"%s",
				TCHAR_TO_UTF8(*DataRow->ItemID.ToString())
			);

			// ----------------------------------------------------
			// 2. 아이템 이름
			// ----------------------------------------------------

			ImGui::TableSetColumnIndex(1);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);

			ImGui::TextColored(
				ImVec4(0.5f, 1.0f, 0.5f, 1.0f),
				"%s",
				TCHAR_TO_UTF8(*DataRow->ItemText.Name.ToString())
			);

			// ----------------------------------------------------
			// 3. 설명
			// ----------------------------------------------------

			ImGui::TableSetColumnIndex(2);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);

			ImGui::TextWrapped(
				"%s",
				TCHAR_TO_UTF8(*DataRow->ItemText.Description.ToString())
			);

			// ----------------------------------------------------
			// 4. 아이템 타입
			// DataTable 자체가 StatItem 전용이므로 enum 판별 불필요
			// ----------------------------------------------------

			ImGui::TableSetColumnIndex(3);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);

			ImGui::TextWrapped("Stat Item");

			// ----------------------------------------------------
			// 5. 지급 버튼
			// ----------------------------------------------------

			ImGui::TableSetColumnIndex(4);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);

			bool bIsPetValid = SelectedPet.IsValid();

			if (!bIsPetValid)
			{
				ImGui::BeginDisabled();
			}

			ImGui::PushStyleColor(
				ImGuiCol_Button,
				ImVec4(0.15f, 0.5f, 0.7f, 1.0f)
			);

			ImGui::PushStyleColor(
				ImGuiCol_ButtonHovered,
				ImVec4(0.2f, 0.6f, 0.85f, 1.0f)
			);

			ImGui::PushStyleColor(
				ImGuiCol_ButtonActive,
				ImVec4(0.1f, 0.4f, 0.6f, 1.0f)
			);

			if (ImGui::Button(
				"지급 (Give)",
				ImVec2(-FLT_MIN, 36.0f)))
			{
				UPetStatItemDataAsset* ItemAsset =
					Cast<UPetStatItemDataAsset>(
						DataRow->ItemDataAsset.LoadSynchronous()
					);

				if (ItemAsset)
				{
					APlayerCharacter* PlayerChar =
						Cast<APlayerCharacter>(
							GetWorld()
							->GetFirstPlayerController()
							->GetPawn()
						);

					UPetManagerComponent* PetManager =
						PlayerChar
						? PlayerChar->FindComponentByClass<
						UPetManagerComponent>()
						: nullptr;

					if (PetManager)
					{
						PetManager->GiveItemToPet(
							SelectedPet.Get(),
							ItemAsset
						);

						UE_LOG(
							LogTemp,
							Log,
							TEXT("[Debug] %s 지급 완료"),
							*ItemAsset->ItemName.ToString()
						);
					}
				}
			}

			ImGui::PopStyleColor(3);

			if (!bIsPetValid)
			{
				ImGui::EndDisabled();
			}

			ImGui::PopID();
		}

		ImGui::EndTable();
	}
}

void APetItemDebugActor::DrawWeaponItemsTab()
{
	if (AllWeaponItemDataRows.IsEmpty())
	{
		ImGui::TextDisabled("로드된 무기 아이템이 없습니다.");
		return;
	}

	const ImGuiTableFlags TableFlags =
		ImGuiTableFlags_Borders |
		ImGuiTableFlags_RowBg |
		ImGuiTableFlags_Resizable |
		ImGuiTableFlags_SizingStretchProp |
		ImGuiTableFlags_PadOuterX;

	if (ImGui::BeginTable("WeaponItemsTable", 5, TableFlags))
	{
		ImGui::TableSetupColumn(
			"아이템 ID",
			ImGuiTableColumnFlags_WidthStretch,
			1.0f
		);

		ImGui::TableSetupColumn(
			"Item Name",
			ImGuiTableColumnFlags_WidthStretch,
			2.0f
		);

		ImGui::TableSetupColumn(
			"Item Description",
			ImGuiTableColumnFlags_WidthStretch,
			1.0f
		);

		ImGui::TableSetupColumn(
			"Item Type",
			ImGuiTableColumnFlags_WidthStretch,
			3.0f
		);

		ImGui::TableSetupColumn(
			"액션",
			ImGuiTableColumnFlags_WidthFixed,
			100.0f
		);

		ImGui::TableHeadersRow();

		for (FWeaponItemDataRow* DataRow : AllWeaponItemDataRows)
		{
			if (!DataRow)
			{
				continue;
			}

			ImGui::PushID(DataRow);
			ImGui::TableNextRow();

			// ----------------------------------------------------
			// 1. 아이템 ID
			// ----------------------------------------------------

			ImGui::TableSetColumnIndex(0);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);

			ImGui::TextColored(
				ImVec4(0.5f, 1.0f, 0.5f, 1.0f),
				"%s",
				TCHAR_TO_UTF8(*DataRow->ItemID.ToString())
			);

			// ----------------------------------------------------
			// 2. 아이템 이름
			// ----------------------------------------------------

			ImGui::TableSetColumnIndex(1);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);

			ImGui::TextColored(
				ImVec4(0.5f, 1.0f, 0.5f, 1.0f),
				"%s",
				TCHAR_TO_UTF8(*DataRow->ItemText.Name.ToString())
			);

			// ----------------------------------------------------
			// 3. 설명
			// ----------------------------------------------------

			ImGui::TableSetColumnIndex(2);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);

			ImGui::TextWrapped(
				"%s",
				TCHAR_TO_UTF8(*DataRow->ItemText.Description.ToString())
			);

			// ----------------------------------------------------
			// 4. 아이템 타입
			// DataTable 자체가 WeaponItem 전용이므로 enum 판별 불필요
			// ----------------------------------------------------

			ImGui::TableSetColumnIndex(3);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);

			ImGui::TextWrapped("Weapon Item");

			// ----------------------------------------------------
			// 5. 지급 버튼
			// ----------------------------------------------------

			ImGui::TableSetColumnIndex(4);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);

			bool bIsPetValid = SelectedPet.IsValid();

			if (!bIsPetValid)
			{
				ImGui::BeginDisabled();
			}

			ImGui::PushStyleColor(
				ImGuiCol_Button,
				ImVec4(0.15f, 0.5f, 0.7f, 1.0f)
			);

			ImGui::PushStyleColor(
				ImGuiCol_ButtonHovered,
				ImVec4(0.2f, 0.6f, 0.85f, 1.0f)
			);

			ImGui::PushStyleColor(
				ImGuiCol_ButtonActive,
				ImVec4(0.1f, 0.4f, 0.6f, 1.0f)
			);

			if (ImGui::Button(
				"지급 (Give)",
				ImVec2(-FLT_MIN, 36.0f)))
			{
				UPetProjectileItemDataAsset* ItemAsset =
					Cast<UPetProjectileItemDataAsset>(
						DataRow->ItemDataAsset.LoadSynchronous()
					);

				if (ItemAsset)
				{
					APlayerCharacter* PlayerChar =
						Cast<APlayerCharacter>(
							GetWorld()
							->GetFirstPlayerController()
							->GetPawn()
						);

					UPetManagerComponent* PetManager =
						PlayerChar
						? PlayerChar->FindComponentByClass<
						UPetManagerComponent>()
						: nullptr;

					if (PetManager)
					{
						PetManager->GiveItemToPet(
							SelectedPet.Get(),
							ItemAsset
						);

						UE_LOG(
							LogTemp,
							Log,
							TEXT("[Debug] %s 지급 완료"),
							*ItemAsset->ItemName.ToString()
						);
					}
				}
			}

			ImGui::PopStyleColor(3);

			if (!bIsPetValid)
			{
				ImGui::EndDisabled();
			}

			ImGui::PopID();
		}

		ImGui::EndTable();
	}
}

#endif
