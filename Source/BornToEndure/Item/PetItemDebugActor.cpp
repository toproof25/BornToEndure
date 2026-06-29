#include "Item/PetItemDebugActor.h"

#include "Component/PetManagerComponent.h"
#include "Character/Pet/PetCompanionCharacter.h"
#include "Character/Player/PlayerCharacter.h"
#include "Stat/PetStatTypes.h"
#include "Engine/AssetManager.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "Data/DataTableRow/ItemDataRow.h"

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
		FImGuiDelegate Delegate = FImGuiDelegate::CreateUObject(this, &APetItemDebugActor::RenderImGui);
		ImGuiDelegateHandle = FImGuiModule::Get().AddWorldImGuiDelegate(Delegate);
	}
#endif
	//LoadAllPetItemsAsync();


	// DataTable 테스트 (정상 로드 확인)
	if (ItemDataTable)
	{
		TArray<FName> RowNames = ItemDataTable->GetRowNames();
		UE_LOG(LogTemp, Warning, TEXT("[PetItemDebug] --- 데이터 테이블 내부 행 목록 시작 (총 %d개) ---"), RowNames.Num());

		for (const FName& Name : RowNames)
		{
			// 이름 양옆에 대괄호[]를 붙여 공백이 있는지 시각적으로 확인합니다.
			UE_LOG(LogTemp, Log, TEXT("[PetItemDebug] 행 이름 발견: [%s]"), *Name.ToString());
		}

		UE_LOG(LogTemp, Warning, TEXT("[PetItemDebug] --- 데이터 테이블 내부 행 목록 끝 ---"));

		// 디버그용 아이템 지급 Chach
		TArray<FItemDataRow*> AllRows;
		ItemDataTable->GetAllRows<FItemDataRow>(TEXT("Debug Actor LoadAllPetItemsAsync"), AllRows);

		AllStatItemDataRows = AllRows.FilterByPredicate([](FItemDataRow* Row) { return Row->ItemType == EItemType::Stat; });
		AllProjectileItemDataRows = AllRows.FilterByPredicate([](FItemDataRow* Row) { return Row->ItemType == EItemType::Projectile; });
		bIsLoadingAssets = true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[PetItemDebug] ItemDataTable이 null입니다! 에디터에서 에셋이 할당되었는지 확인하세요."));
		return;
	}


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
		UE_LOG(LogTemp, Warning, TEXT("[PetItemDebug] 발견된 펫 아이템 에셋이 없습니다. DefaultGame.ini를 확인하세요."));
		return;
	}

	AM.LoadPrimaryAssets(ItemIds, TArray<FName>(),
		FStreamableDelegate::CreateUObject(this, &APetItemDebugActor::OnItemsLoaded, ItemIds));
}

void APetItemDebugActor::OnItemsLoaded(TArray<FPrimaryAssetId> LoadedIds)
{
	UAssetManager& AM = UAssetManager::Get();

	LoadedStatItems.Empty();
	LoadedProjectileItems.Empty();

	for (const FPrimaryAssetId& Id : LoadedIds)
	{
		UObject* LoadedObj = AM.GetPrimaryAssetObject(Id);

		if (UPetStatItemDataAsset* StatItem = Cast<UPetStatItemDataAsset>(LoadedObj))
		{
			LoadedStatItems.Add(StatItem);
		}
		else if (UPetProjectileItemDataAsset* ProjItem = Cast<UPetProjectileItemDataAsset>(LoadedObj))
		{
			LoadedProjectileItems.Add(ProjItem);
		}
	}

	bIsLoadingAssets = false;
	UE_LOG(LogTemp, Log, TEXT("[PetItemDebug] 아이템 로드 완료! Stat: %d개, Projectile: %d개"), LoadedStatItems.Num(), LoadedProjectileItems.Num());



}

#if !UE_BUILD_SHIPPING
void APetItemDebugActor::RenderImGui()
{
	// 1. 미니 컨트롤 패널 (최소화 상태)
	if (!bIsWindowOpen)
	{
		ImGui::SetNextWindowBgAlpha(0.8f);
		if (ImGui::Begin("Pet Debug Tools", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings))
		{
			if (ImGui::Button("펫 아이템 툴 열기 (Open Debugger)", ImVec2(250, 40)))
			{
				bIsWindowOpen = true;
			}
		}
		ImGui::End();
		return;
	}

	// 2. 메인 디버거 윈도우
	ImGui::SetNextWindowSize(ImVec2(1000, 600), ImGuiCond_FirstUseEver); // 텍스처를 위해 가로폭을 넉넉히 확보
	if (ImGui::Begin("펫 아이템 실시간 관리자 (Pet Item Debugger)", &bIsWindowOpen, ImGuiWindowFlags_None))
	{
		APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
		UPetManagerComponent* PetManager = PlayerChar ? PlayerChar->FindComponentByClass<UPetManagerComponent>() : nullptr;

		if (!PetManager)
		{
			ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "[오류] 플레이어 캐릭터 또는 PetManagerComponent를 찾을 수 없습니다.");
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

			if (ImGui::BeginTabItem("발사체 아이템 (Projectile Items)"))
			{
				DrawProjectileItemsTab();
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}
	ImGui::End();
}

void APetItemDebugActor::DrawPetSelectionCombo(UPetManagerComponent* PetManager)
{
	const TArray<TObjectPtr<APetCompanionCharacter>>& OwnedPets = PetManager->GetPetList();

	ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "타겟 펫 선택 (Target Pet) :");
	ImGui::SameLine();

	APetCompanionCharacter* CurrentPet = SelectedPet.Get();

	if (CurrentPet && !OwnedPets.Contains(CurrentPet))
	{
		CurrentPet = nullptr;
		SelectedPet.Reset();
	}

	FString PreviewName = CurrentPet ? CurrentPet->GetName() : TEXT("--- 펫을 선택하세요 ---");

	ImGui::PushItemWidth(350.0f);
	if (ImGui::BeginCombo("##TargetPetCombo", TCHAR_TO_UTF8(*PreviewName)))
	{
		if (OwnedPets.IsEmpty())
		{
			ImGui::Selectable("소유 중인 펫이 없습니다.", false, ImGuiSelectableFlags_Disabled);
		}
		else
		{
			for (int32 i = 0; i < OwnedPets.Num(); ++i)
			{
				APetCompanionCharacter* Pet = OwnedPets[i].Get();
				if (!Pet) continue;

				bool bIsSelected = (CurrentPet == Pet);
				FString DisplayName = FString::Printf(TEXT("[%d] %s"), i, *Pet->GetName());

				if (ImGui::Selectable(TCHAR_TO_UTF8(*DisplayName), bIsSelected))
				{
					SelectedPet = Pet;
				}

				if (bIsSelected) ImGui::SetItemDefaultFocus();
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

	// 🌟 가독성을 위한 6열 테이블 구성
	const ImGuiTableFlags TableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_PadOuterX;
	if (ImGui::BeginTable("StatItemsTable", 6, TableFlags))
	{
		// 1. 아이콘: 42픽셀 고정
		//ImGui::TableSetupColumn("아이콘", ImGuiTableColumnFlags_WidthFixed, 42.0f);

		// 2. 가변 영역 (비율 할당)
		ImGui::TableSetupColumn("아이템 ID", ImGuiTableColumnFlags_WidthStretch, 1.0f); // 1 비율
		ImGui::TableSetupColumn("Item Name", ImGuiTableColumnFlags_WidthStretch, 2.0f);       // 2 비율
		ImGui::TableSetupColumn("Item Description", ImGuiTableColumnFlags_WidthStretch, 1.0f); // 1 비율
		ImGui::TableSetupColumn("Item Type", ImGuiTableColumnFlags_WidthStretch, 3.0f); // 3 비율 (가장 넓음!)

		// 3. 버튼: 100픽셀 고정
		ImGui::TableSetupColumn("액션", ImGuiTableColumnFlags_WidthFixed, 100.0f);

		ImGui::TableHeadersRow();
		
		for (FItemDataRow* DataRow : AllStatItemDataRows)
		{
			if (!DataRow) continue;
			ImGui::PushID(DataRow);
			ImGui::TableNextRow();

			// ----------------------------------------------------
			// 1. 아이템 ID
			// ----------------------------------------------------
			ImGui::TableSetColumnIndex(0);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
			ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "%s", TCHAR_TO_UTF8(*DataRow->ItemID.ToString()));

			// ----------------------------------------------------
			// 2. 아이템 이름
			// ----------------------------------------------------
			ImGui::TableSetColumnIndex(1);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
			ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "%s", TCHAR_TO_UTF8(*DataRow->ItemText.Name.ToString()));

			// ----------------------------------------------------
			// 3. 설명
			// ----------------------------------------------------
			ImGui::TableSetColumnIndex(2);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
			ImGui::TextWrapped("%s", TCHAR_TO_UTF8(*DataRow->ItemText.Description.ToString()));

			// ----------------------------------------------------
			// 4. 아이템 타입
			// ----------------------------------------------------
			ImGui::TableSetColumnIndex(3);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
			FString Type = DataRow->ItemType == EItemType::Stat ? "Stat Item" : "Projectile Item";
			ImGui::TextWrapped("%s", TCHAR_TO_UTF8(*Type));

			// ----------------------------------------------------
			// 5. 지급 버튼 (높이를 36으로 맞추어 아이콘과 세로 정렬 동기화)
			// ----------------------------------------------------
			ImGui::TableSetColumnIndex(4);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);

			bool bIsPetValid = SelectedPet.IsValid();
			if (!bIsPetValid) ImGui::BeginDisabled();

			// 🌟 지급 버튼 디자인 개선 (블루/그린 계열)
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.5f, 0.7f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.6f, 0.85f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.4f, 0.6f, 1.0f));

			if (ImGui::Button("지급 (Give)", ImVec2(-FLT_MIN, 36.0f)))
			{

				UPetStatItemDataAsset* ItemAsset = Cast<UPetStatItemDataAsset>(DataRow->ItemDataAsset.LoadSynchronous());

				APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
				UPetManagerComponent* PetManager = PlayerChar->FindComponentByClass<UPetManagerComponent>();
				if (PetManager)
				{
					PetManager->GiveItemToPet(SelectedPet.Get(), ItemAsset);
					UE_LOG(LogTemp, Log, TEXT("[Debug] %s 지급 완료"), *ItemAsset->ItemName.ToString());
				}
			}

			ImGui::PopStyleColor(3);
			if (!bIsPetValid) ImGui::EndDisabled();

			ImGui::PopID();
		}
		ImGui::EndTable();
	}
}

void APetItemDebugActor::DrawProjectileItemsTab()
{
	if (AllProjectileItemDataRows.IsEmpty())
	{
		ImGui::TextDisabled("로드된 발사체 아이템이 없습니다.");
		return;
	}

	const ImGuiTableFlags TableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_PadOuterX;

	if (ImGui::BeginTable("ProjItemsTable", 6, TableFlags))
	{
		// 1. 아이콘: 42픽셀 고정
		//ImGui::TableSetupColumn("아이콘", ImGuiTableColumnFlags_WidthFixed, 42.0f);

		// 2. 가변 영역 (비율 할당)
		ImGui::TableSetupColumn("아이템 ID", ImGuiTableColumnFlags_WidthStretch, 1.0f); // 1 비율
		ImGui::TableSetupColumn("Item Name", ImGuiTableColumnFlags_WidthStretch, 2.0f);       // 2 비율
		ImGui::TableSetupColumn("Item Description", ImGuiTableColumnFlags_WidthStretch, 1.0f); // 1 비율
		ImGui::TableSetupColumn("Item Type", ImGuiTableColumnFlags_WidthStretch, 3.0f); // 3 비율 (가장 넓음!)

		// 3. 버튼: 100픽셀 고정
		ImGui::TableSetupColumn("액션", ImGuiTableColumnFlags_WidthFixed, 100.0f);

		ImGui::TableHeadersRow();

		for (FItemDataRow* DataRow : AllProjectileItemDataRows)
		{
			if (!DataRow) continue;
			ImGui::PushID(DataRow);
			ImGui::TableNextRow();

			// ----------------------------------------------------
			// 1. 아이템 ID
			// ----------------------------------------------------
			ImGui::TableSetColumnIndex(0);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
			ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "%s", TCHAR_TO_UTF8(*DataRow->ItemID.ToString()));

			// ----------------------------------------------------
			// 2. 아이템 이름
			// ----------------------------------------------------
			ImGui::TableSetColumnIndex(1);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
			ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "%s", TCHAR_TO_UTF8(*DataRow->ItemText.Name.ToString()));

			// ----------------------------------------------------
			// 3. 설명
			// ----------------------------------------------------
			ImGui::TableSetColumnIndex(2);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
			ImGui::TextWrapped("%s", TCHAR_TO_UTF8(*DataRow->ItemText.Description.ToString()));

			// ----------------------------------------------------
			// 4. 아이템 타입
			// ----------------------------------------------------
			ImGui::TableSetColumnIndex(3);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
			FString Type = DataRow->ItemType == EItemType::Stat ? "Stat Item" : "Projectile Item";
			ImGui::TextWrapped("%s", TCHAR_TO_UTF8(*Type));

			// ----------------------------------------------------
			// 5. 지급 버튼 (높이를 36으로 맞추어 아이콘과 세로 정렬 동기화)
			// ----------------------------------------------------
			ImGui::TableSetColumnIndex(4);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);

			bool bIsPetValid = SelectedPet.IsValid();
			if (!bIsPetValid) ImGui::BeginDisabled();

			// 🌟 지급 버튼 디자인 개선 (블루/그린 계열)
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.5f, 0.7f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.6f, 0.85f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.4f, 0.6f, 1.0f));

			if (ImGui::Button("지급 (Give)", ImVec2(-FLT_MIN, 36.0f)))
			{

				UPetProjectileItemDataAsset* ItemAsset = Cast<UPetProjectileItemDataAsset>(DataRow->ItemDataAsset.LoadSynchronous());

				APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
				UPetManagerComponent* PetManager = PlayerChar->FindComponentByClass<UPetManagerComponent>();
				if (PetManager)
				{
					PetManager->GiveItemToPet(SelectedPet.Get(), ItemAsset);
					UE_LOG(LogTemp, Log, TEXT("[Debug] %s 지급 완료"), *ItemAsset->ItemName.ToString());
				}
			}

			ImGui::PopStyleColor(3);
			if (!bIsPetValid) ImGui::EndDisabled();

			ImGui::PopID();
		}
		ImGui::EndTable();
	}
}

#endif