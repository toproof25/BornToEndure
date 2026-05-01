#include "Item/PetItemDebugActor.h"

#include "Component/PetManagerComponent.h"
#include "Character/Pet/PetCompanionCharacter.h"
#include "Character/Player/PlayerCharacter.h"
#include "Stat/PetStatTypes.h"
#include "Engine/AssetManager.h"
#include "Engine/World.h"

#if !UE_BUILD_SHIPPING
#include "imgui.h"
#include "ImGuiModule.h"
#include "ImGuiTextureHandle.h" // 🌟 UnrealImGui 텍스처 핸들 공식 헤더
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

	LoadAllPetItemsAsync();
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

		if (bIsLoadingAssets)
		{
			ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "데이터 애셋 로딩 중...");
			ImGui::End();
			return;
		}

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
	const TArray<TObjectPtr<APetCompanionCharacter>>& OwnedPets = PetManager->GetOwnedPets();

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
	if (LoadedStatItems.IsEmpty())
	{
		ImGui::TextDisabled("로드된 스탯 아이템이 없습니다.");
		return;
	}

	// 🌟 가독성을 위한 6열 테이블 구성
	const ImGuiTableFlags TableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_PadOuterX;
	if (ImGui::BeginTable("StatItemsTable", 6, TableFlags))
	{
		// 1. 아이콘: 42픽셀 고정
		ImGui::TableSetupColumn("아이콘", ImGuiTableColumnFlags_WidthFixed, 42.0f);

		// 2. 가변 영역 (비율 할당)
		ImGui::TableSetupColumn("아이템 이름", ImGuiTableColumnFlags_WidthStretch, 1.0f); // 1 비율
		ImGui::TableSetupColumn("설명", ImGuiTableColumnFlags_WidthStretch, 2.0f);       // 2 비율
		ImGui::TableSetupColumn("시너지 태그", ImGuiTableColumnFlags_WidthStretch, 1.0f); // 1 비율
		ImGui::TableSetupColumn("스탯 변화량", ImGuiTableColumnFlags_WidthStretch, 3.0f); // 3 비율 (가장 넓음!)

		// 3. 버튼: 100픽셀 고정
		ImGui::TableSetupColumn("액션", ImGuiTableColumnFlags_WidthFixed, 100.0f);

		ImGui::TableHeadersRow();

		for (UPetStatItemDataAsset* Item : LoadedStatItems)
		{
			if (!Item) continue;

			ImGui::PushID(Item);
			ImGui::TableNextRow();

			// ----------------------------------------------------
			// 1. 아이콘 (GC 방지 및 ImGui 렌더링 완벽 대응)
			// ----------------------------------------------------
			ImGui::TableSetColumnIndex(0);

			if (UTexture2D* IconTex = Item->ItemIcon.LoadSynchronous())
			{
				// 🌟 핵심 1: 언리얼 GC가 텍스처를 메모리에서 날리지 못하게 멱살을 잡습니다.
				CachedIcons.AddUnique(IconTex);

				// 🌟 핵심 2: 기존에 망가진 캐시와 이름이 겹치지 않도록, 고유한 접두사(ImGuiTex_)를 붙여 새롭게 등록합니다.
				FName TexName = FName(*FString::Printf(TEXT("ImGuiTex_%s"), *IconTex->GetName()));
				FImGuiTextureHandle TexHandle = FImGuiModule::Get().FindTextureHandle(TexName);

				if (!TexHandle.IsValid())
				{
					// 텍스처를 ImGui 모듈에 등록
					TexHandle = FImGuiModule::Get().RegisterTexture(TexName, IconTex);
				}

				if (TexHandle.IsValid())
				{
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);
					// 흰색 틴트(1,1,1,1)를 적용하여 원본 색상과 투명도(Alpha)를 유지합니다.
					ImGui::Image(TexHandle.GetTextureId(), ImVec2(36, 36), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1));
				}
			}
			else
			{
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
				ImGui::TextDisabled(" N/A ");
			}

			// ----------------------------------------------------
			// 2. 아이템 이름 (세로 중앙 정렬 느낌을 위해 Spacing 추가)
			// ----------------------------------------------------
			ImGui::TableSetColumnIndex(1);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
			ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "%s", TCHAR_TO_UTF8(*Item->ItemName.ToString()));

			// ----------------------------------------------------
			// 3. 설명
			// ----------------------------------------------------
			ImGui::TableSetColumnIndex(2);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
			ImGui::TextWrapped("%s", TCHAR_TO_UTF8(*Item->ItemDescription.ToString()));

			// ----------------------------------------------------
			// 4. 시너지 태그
			// ----------------------------------------------------
			ImGui::TableSetColumnIndex(3);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
			FString TagsStr = Item->SynergyTags.ToStringSimple();
			if (TagsStr.IsEmpty()) ImGui::TextDisabled("없음");
			else ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%s", TCHAR_TO_UTF8(*TagsStr));

			// ----------------------------------------------------
			// 5. 스탯 변화량 (한국어 DisplayName 적용)
			// ----------------------------------------------------
			ImGui::TableSetColumnIndex(4);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);

			for (const FStatModifier& Mod : Item->StatModifiers)
			{
				// 🌟 템플릿 함수를 사용하여 한국어 DisplayName을 가져옵니다.
				FString StatName = GetEnumDisplayName<EPetStatType>(TEXT("EPetStatType"), Mod.StatType);
				FString ModName = GetEnumDisplayName<EStatModifierType>(TEXT("EStatModifierType"), Mod.ModType);

				ImVec4 ValColor = Mod.Value >= 0 ? ImVec4(0.3f, 0.9f, 0.3f, 1.0f) : ImVec4(0.9f, 0.3f, 0.3f, 1.0f);

				// 출력 예시: [고정값 추가] 공격력 : 10.0
				ImGui::TextDisabled("[%s]", TCHAR_TO_UTF8(*ModName));
				ImGui::SameLine();
				ImGui::Text("%s : ", TCHAR_TO_UTF8(*StatName));
				ImGui::SameLine();

				// 퍼센트 연산일 경우 % 기호를 붙여주면 더 직관적입니다.
				if (Mod.ModType == EStatModifierType::Multiplicative)
				{
					ImGui::TextColored(ValColor, "%.1f%%", Mod.Value * 100.f);
				}
				else
				{
					ImGui::TextColored(ValColor, "%.1f", Mod.Value);
				}
			}

			// ----------------------------------------------------
			// 6. 지급 버튼 (높이를 36으로 맞추어 아이콘과 세로 정렬 동기화)
			// ----------------------------------------------------
			ImGui::TableSetColumnIndex(5);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);

			bool bIsPetValid = SelectedPet.IsValid();
			if (!bIsPetValid) ImGui::BeginDisabled();

			// 🌟 지급 버튼 디자인 개선 (블루/그린 계열)
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.5f, 0.7f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.6f, 0.85f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.4f, 0.6f, 1.0f));

			if (ImGui::Button("지급 (Give)", ImVec2(-FLT_MIN, 36.0f)))
			{
				APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
				UPetManagerComponent* PetManager = PlayerChar->FindComponentByClass<UPetManagerComponent>();
				if (PetManager)
				{
					PetManager->GiveItemToPet(SelectedPet.Get(), Item);
					UE_LOG(LogTemp, Log, TEXT("[Debug] %s 지급 완료"), *Item->ItemName.ToString());
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
	if (LoadedProjectileItems.IsEmpty())
	{
		ImGui::TextDisabled("로드된 발사체 아이템이 없습니다.");
		return;
	}

	const ImGuiTableFlags TableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_PadOuterX;

	if (ImGui::BeginTable("ProjItemsTable", 6, TableFlags))
	{
		// 1. 아이콘: 42픽셀 고정
		ImGui::TableSetupColumn("아이콘", ImGuiTableColumnFlags_WidthFixed, 42.0f);

		// 2. 가변 영역 (비율 할당)
		ImGui::TableSetupColumn("아이템 이름", ImGuiTableColumnFlags_WidthStretch, 1.0f); // 1 비율
		ImGui::TableSetupColumn("설명", ImGuiTableColumnFlags_WidthStretch, 2.0f);       // 2 비율
		ImGui::TableSetupColumn("시너지 태그", ImGuiTableColumnFlags_WidthStretch, 1.0f); // 1 비율
		ImGui::TableSetupColumn("발사체 옵션", ImGuiTableColumnFlags_WidthStretch, 3.0f); // 3 비율 (가장 넓음!)

		// 3. 버튼: 100픽셀 고정
		ImGui::TableSetupColumn("액션", ImGuiTableColumnFlags_WidthFixed, 100.0f);

		ImGui::TableHeadersRow();

		for (UPetProjectileItemDataAsset* Item : LoadedProjectileItems)
		{
			if (!Item) continue;

			ImGui::PushID(Item);
			ImGui::TableNextRow();

			// ----------------------------------------------------
			// 1. 아이콘 (GC 방지 및 ImGui 렌더링 완벽 대응)
			// ----------------------------------------------------
			ImGui::TableSetColumnIndex(0);

			if (UTexture2D* IconTex = Item->ItemIcon.LoadSynchronous())
			{
				// 🌟 핵심 1: 언리얼 GC가 텍스처를 메모리에서 날리지 못하게 멱살을 잡습니다.
				CachedIcons.AddUnique(IconTex);

				// 🌟 핵심 2: 기존에 망가진 캐시와 이름이 겹치지 않도록, 고유한 접두사(ImGuiTex_)를 붙여 새롭게 등록합니다.
				FName TexName = FName(*FString::Printf(TEXT("ImGuiTex_%s"), *IconTex->GetName()));
				FImGuiTextureHandle TexHandle = FImGuiModule::Get().FindTextureHandle(TexName);

				if (!TexHandle.IsValid())
				{
					// 텍스처를 ImGui 모듈에 등록
					TexHandle = FImGuiModule::Get().RegisterTexture(TexName, IconTex);
				}

				if (TexHandle.IsValid())
				{
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);
					// 흰색 틴트(1,1,1,1)를 적용하여 원본 색상과 투명도(Alpha)를 유지합니다.
					ImGui::Image(TexHandle.GetTextureId(), ImVec2(36, 36), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1));
				}
			}
			else
			{
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
				ImGui::TextDisabled(" N/A ");
			}

			// ----------------------------------------------------
			// 2. 아이템 이름
			// ----------------------------------------------------
			ImGui::TableSetColumnIndex(1);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
			ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", TCHAR_TO_UTF8(*Item->ItemName.ToString()));

			// ----------------------------------------------------
			// 3. 설명
			// ----------------------------------------------------
			ImGui::TableSetColumnIndex(2);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
			ImGui::TextWrapped("%s", TCHAR_TO_UTF8(*Item->ItemDescription.ToString()));

			// ----------------------------------------------------
			// 4. 시너지 태그
			// ----------------------------------------------------
			ImGui::TableSetColumnIndex(3);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
			FString TagsStr = Item->SynergyTags.ToStringSimple();
			if (TagsStr.IsEmpty()) ImGui::TextDisabled("없음");
			else ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%s", TCHAR_TO_UTF8(*TagsStr));

			// ----------------------------------------------------
			// 5. 발사체 옵션 상세
			// ----------------------------------------------------
			ImGui::TableSetColumnIndex(4);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);
			const FProjectileModifierData& Mod = Item->ProjectileModifier;

			FString PatternName = GetEnumDisplayName<EProjectilePattern>(TEXT("EProjectilePattern"), Mod.Pattern);
			ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "패턴: %s", TCHAR_TO_UTF8(*PatternName));

			if (Mod.ProjectileCountAdd > 0) ImGui::Text("개수 추가: +%d", Mod.ProjectileCountAdd);
			if (Mod.SizeMultiplier != 1.0f) ImGui::Text("크기 배율: x%.2f", Mod.SizeMultiplier);
			if (Mod.SpeedMultiplier != 1.0f) ImGui::Text("속도 배율: x%.2f", Mod.SpeedMultiplier);

			if (!Mod.OverrideProjectileClass.IsNull())
			{
				ImGui::TextColored(ImVec4(0.8f, 0.4f, 1.0f, 1.0f), "[클래스 덮어쓰기 적용됨]");
			}

			// ----------------------------------------------------
			// 6. 지급 버튼
			// ----------------------------------------------------
			ImGui::TableSetColumnIndex(5);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);

			bool bIsPetValid = SelectedPet.IsValid();
			if (!bIsPetValid) ImGui::BeginDisabled();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.3f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.3f, 0.4f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.15f, 0.2f, 1.0f));

			if (ImGui::Button("지급 (Give)", ImVec2(-FLT_MIN, 36.0f)))
			{
				APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
				UPetManagerComponent* PetManager = PlayerChar->FindComponentByClass<UPetManagerComponent>();
				if (PetManager)
				{
					PetManager->GiveItemToPet(SelectedPet.Get(), Item);
					UE_LOG(LogTemp, Log, TEXT("[Debug] %s 지급 완료"), *Item->ItemName.ToString());
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