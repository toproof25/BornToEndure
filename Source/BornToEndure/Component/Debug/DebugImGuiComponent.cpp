// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugImGuiComponent.h"

#include "Data/DebugFont/KoreanFont.h"
#include "imgui.h"
#include "ImGuiModule.h"     
#include "ImGuiDelegates.h" 

#include "Character/Player/PlayerCharacter.h"
#include "Character/Player/PlayerAnimInstance.h"
#include "Component/StatComponent.h"
#include "Item/Weapon/BaseWeapon.h"
#include "Item/Weapon/FireWeapon.h"
#include "Engine/World.h"

#include "Subsystem/ObjectPoolSubsystem.h"
#include "Subsystem/EffectSubsystem.h"

#include "GameFramework/Character.h"

// Camera 관련 헤더 포함
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Component/PetManagerComponent.h"
#include "Character/Pet/PetCompanionCharacter.h"
#include "Component/PetCombatComponent.h"
#include "Component/PetStatComponent.h"
#include "Component/PetItemComponent.h"
#include "Data/PetItemDataAsset.h"
#include "Data/PetSynergyDataAsset.h"
#include "Data/PetWeaponItemDataAsset.h"
#include "Stat/PetStatTypes.h"
#include "Component/PlayerExperienceComponent.h"
#include "PlayerState/CombatPlayerState.h"

#include "Component/EnemyDetectorComponent.h" 
#include "Character/Enemy/BaseEnemyCharacter.h" 

#include "Engine/DataTable.h"
#include "Data/DataTableRow/ItemDataRow.h"

namespace ImGuiUtils
{
	// Key-Value 테이블 행 출력
	void DrawRowText(const char* Label, const char* ValueFormat, ...)
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::TextDisabled("%s", Label); // 키 값은 약간 어둡게

		ImGui::TableSetColumnIndex(1);
		va_list args;
		va_start(args, ValueFormat);
		ImGui::TextV(ValueFormat, args);
		va_end(args);
	}

	// Bool 상태를 색상으로 구분하여 출력 (True = Green, False = Gray)
	void DrawRowBool(const char* Label, bool bValue)
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::TextDisabled("%s", Label);

		ImGui::TableSetColumnIndex(1);
		if (bValue)
			ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "True");
		else
			ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "False");
	}

	// FVector를 R, G, B 색상으로 구분하여 출력
	void DrawRowVector(const char* Label, const FVector& Vec)
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::TextDisabled("%s", Label);

		ImGui::TableSetColumnIndex(1);
		ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "X:%.2f", Vec.X); ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "Y:%.2f", Vec.Y); ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.3f, 0.5f, 1.0f, 1.0f), "Z:%.2f", Vec.Z);
	}
}

UDebugImGuiComponent::UDebugImGuiComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

void UDebugImGuiComponent::InitializeComponent()
{
	Super::InitializeComponent();

	LoadKoreanFontToImGui();

	AActor* Owner = GetOwner();
	if (!Owner) return;

	// Camera
	SpringArmComp = Owner->FindComponentByClass<USpringArmComponent>();
	CameraComp = Owner->FindComponentByClass<UCameraComponent>();

	// Player Component
	PlayerCharacter = Cast<APlayerCharacter>(Owner);

	USkeletalMeshComponent* SkeletalMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
	if (SkeletalMesh && SkeletalMesh->GetAnimInstance())
	{
		PlayerAnimInstance = Cast<UPlayerAnimInstance>(SkeletalMesh->GetAnimInstance());
	}

	PlayerStatComponent = Owner->FindComponentByClass<UStatComponent>();
	PlayerMovementComponent = Owner->FindComponentByClass<UCharacterMovementComponent>();
	PetManagerComponent = Owner->FindComponentByClass<UPetManagerComponent>();
	EnemyDetectorComponent = Owner->FindComponentByClass<UEnemyDetectorComponent>(); 
	
}

void UDebugImGuiComponent::BeginPlay()
{
	Super::BeginPlay();

	if (FImGuiModule::IsAvailable() && IsValid(GetWorld()))
	{
		LoadKoreanFontToImGui();
		FImGuiDelegate Delegate = FImGuiDelegate::CreateUObject(this, &UDebugImGuiComponent::DebugDrawPlayerInfo);
		ImGuiDelegateHandle = FImGuiModule::Get().AddWorldImGuiDelegate(GetWorld(), Delegate);
		UE_LOG(LogTemp, Log, TEXT("ImGui Delegate subscribed!"));
	}

	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (OwningPawn)
	{
		ACombatPlayerState* PS = OwningPawn->GetPlayerState<ACombatPlayerState>();
		if (PS)
		{
			PlayerExperienceComponent = PS->FindComponentByClass<UPlayerExperienceComponent>();
		}
	}
}

void UDebugImGuiComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (FImGuiModule::IsAvailable() && ImGuiDelegateHandle.IsValid())
	{
		FImGuiModule::Get().RemoveImGuiDelegate(ImGuiDelegateHandle);
	}
	ImGuiDelegateHandle.Reset();
	InspectedPet.Reset();
	PetActionResult.Reset();

	Super::EndPlay(EndPlayReason);
}

void UDebugImGuiComponent::LoadKoreanFontToImGui()
{
	if (!FImGuiModule::IsAvailable() || ImGui::GetCurrentContext() == nullptr) return;
	if (FImGuiModule::Get().GetProperties().GetCustomFonts().Contains(FName(TEXT("KoreanFont")))) return;

	// 1. ImFontConfig 객체 할당 (플러그인 수명 관리에 맞춰 TSharedPtr 사용)
	TSharedPtr<ImFontConfig> KoreanFontConfig = MakeShareable(new ImFontConfig());

	if (KoreanFontConfig.IsValid())
	{
		// 2. 한국어 글리프(문자) 범위 할당 [매우 중요]
		// ImGui에 내장된 한국어 범위(초성/중성/종성 조합) 포인터를 가져온다.
		static const ImWchar* KoreanRanges = ImGui::GetIO().Fonts->GetGlyphRangesKorean();

		KoreanFontConfig->FontDataOwnedByAtlas = false; // 바이너리 배열(KoreanFont_data)은 전역 정적 메모리이므로 해제 금지
		KoreanFontConfig->FontData = (void*)KoreanFont_data; // 생성된 헤더의 배열 이름
		KoreanFontConfig->FontDataSize = KoreanFont_size;        // 생성된 헤더의 사이즈 상수
		KoreanFontConfig->SizePixels = 18.0f; // 폰트 렌더링 사이즈 (가독성을 위해 16~18 권장)
		KoreanFontConfig->MergeMode = true; // 기본 폰트로 사용할 경우 false. 기존 영문 폰트 위에 병합하려면 true
		KoreanFontConfig->GlyphRanges = KoreanRanges; // 한국어 문자 대역 지정
		KoreanFontConfig->PixelSnapH = true;  // 픽셀 그리드 정렬로 렌더링 품질 향상

		// 3. 플러그인 모듈에 폰트 등록 및 아틀라스 갱신
		FImGuiModule::Get().GetProperties().AddCustomFont("KoreanFont", KoreanFontConfig);
		FImGuiModule::Get().RebuildFontAtlas();
	}
}

void UDebugImGuiComponent::DebugDrawPlayerInfo()
{
	if (!IsValid(GetWorld()) || !IsValid(GetOwner()) || GetOwner()->IsActorBeingDestroyed()) return;

	ImGui::SetNextWindowBgAlpha(0.85f);
	ImGui::SetNextWindowSize(ImVec2(760, 700), ImGuiCond_FirstUseEver);

	if (!ImGui::Begin("종합 디버그 패널 (System Debug Panel)"))
	{
		ImGui::End();
		return;
	}

	ImGui::Text("게임 내 다양한 시스템과 플레이어 상태를 실시간으로 모니터링합니다.");
	ImGui::Separator();
	ImGui::Spacing();

	// ==============================================
	// [수정됨] 플레이어 영역 (항상 최상단에 위치하며 접기/펴기 가능)
	// ==============================================
	ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.4f, 0.4f, 1.0f));
	bool bPlayerSectionOpen = ImGui::CollapsingHeader("[ 플레이어 전용 ] (Player)", ImGuiTreeNodeFlags_DefaultOpen);
	ImGui::PopStyleColor();

	if (bPlayerSectionOpen)
	{
		ImGui::Indent(10.0f); // 내부 요소들 살짝 들여쓰기

		DrawExperienceInfo(); // 🌟 추가: 최상단에 경험치 및 재화 렌더링
		//DrawCameraInfo();
		//DrawPlayerCharacterInfo();
		//DrawAnimationBasic();
		//DrawStatInfo();
		DrawEnemyDetectionInfo();

		//if (PlayerMovementComponent && ImGui::CollapsingHeader("이동 (Movement)"))
		//{
		//	DrawMovement_Basic();
		//	DrawMovement_State();
		//	DrawMovement_Speed();
		//	DrawMovement_Input();
		//	DrawMovement_Physics();
		//}

		ImGui::Unindent(10.0f);
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// ==============================================
	// 탭 바: 그 외 시스템들 (플레이어 밑에 위치)
	// ==============================================
	if (ImGui::BeginTabBar("MainDebugTabBar"))
	{
		if (ImGui::BeginTabItem("펫 & 동료 (Pets)"))
		{
			DrawPetInfo();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("월드 & 메모리 (World & Memory)"))
		{
			DrawSystemAndMemoryInfo();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
}

void UDebugImGuiComponent::DrawCameraInfo()
{
	if (SpringArmComp && ImGui::CollapsingHeader("스프링암 & 카메라", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::BeginTable("CameraTable", 2, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingStretchProp))
		{
			ImGuiUtils::DrawRowVector("스프링암 위치", SpringArmComp->GetComponentLocation());
			if (CameraComp)
			{
				ImGuiUtils::DrawRowVector("카메라 위치", CameraComp->GetComponentLocation());
				ImGuiUtils::DrawRowVector("카메라 회전", CameraComp->GetComponentRotation().Euler());
			}
			ImGui::EndTable();
		}
	}
}

void UDebugImGuiComponent::DrawPlayerCharacterInfo()
{
	if (PlayerCharacter && ImGui::CollapsingHeader("플레이어 캐릭터", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::BeginTable("PlayerTable", 2, ImGuiTableFlags_BordersInnerH))
		{
			ImGuiUtils::DrawRowVector("위치", PlayerCharacter->GetActorLocation());
			ImGuiUtils::DrawRowVector("회전", PlayerCharacter->GetActorRotation().Euler());
			ImGui::EndTable();
		}
	}
}

void UDebugImGuiComponent::DrawAnimationBasic()
{
	if (PlayerAnimInstance && ImGui::CollapsingHeader("애니메이션"))
	{
		if (ImGui::BeginTable("AnimTable", 2, ImGuiTableFlags_BordersInnerH))
		{
			ImGuiUtils::DrawRowText("속도", "%.2f", PlayerAnimInstance->GetGroundSpeed());
			ImGuiUtils::DrawRowBool("낙하 여부", PlayerAnimInstance->GetIsFalling());
			ImGui::EndTable();
		}

		ImGui::Separator();

		if (ImGui::BeginTable("AnimAdvancedTable", 2, ImGuiTableFlags_BordersInnerH))
		{
			ImGuiUtils::DrawRowBool("루트 모션 활성화", PlayerAnimInstance->GetRootMotionMontageInstance() ? true : false);
			ImGuiUtils::DrawRowText("루트 모션 모드", "%d", (int)PlayerAnimInstance->RootMotionMode.GetValue());

			UAnimMontage* CurrentMontage = PlayerAnimInstance->GetCurrentActiveMontage();
			if (CurrentMontage)
			{
				ImGuiUtils::DrawRowText("현재 몽타주", "%s", TCHAR_TO_UTF8(*CurrentMontage->GetName()));
				ImGuiUtils::DrawRowText("재생 위치 (초)", "%.2f / %.2f", PlayerAnimInstance->Montage_GetPosition(CurrentMontage), CurrentMontage->GetPlayLength());

				FName CurrentSection = PlayerAnimInstance->Montage_GetCurrentSection(CurrentMontage);
				ImGuiUtils::DrawRowText("현재 섹션", "%s", TCHAR_TO_UTF8(*CurrentSection.ToString()));
			}
			else
			{
				ImGuiUtils::DrawRowText("현재 몽타주", "없음 (None)");
			}

			float SpeedCurveVal = PlayerAnimInstance->GetCurveValue(FName("Speed"));
			ImGuiUtils::DrawRowText("Speed 커브 값", "%.2f", SpeedCurveVal);

			ImGui::EndTable();
		}
	}
}

void UDebugImGuiComponent::DrawStatInfo()
{
	if (PlayerStatComponent && ImGui::CollapsingHeader("스탯"))
	{
		if (ImGui::BeginTable("StatTable", 2, ImGuiTableFlags_BordersInnerH))
		{
			ImGuiUtils::DrawRowText("기본 속도", "%.2f", PlayerStatComponent->GetBaseWalkSpeed());
			ImGuiUtils::DrawRowText("전력질주 배율", "%.2f", PlayerStatComponent->GetSprintMultiplier());
			ImGuiUtils::DrawRowBool("전력질주 여부", PlayerStatComponent->GetIsSprinting());
			ImGuiUtils::DrawRowText("아이템 추가 속도", "%.2f", PlayerStatComponent->GetItemSpeedBonus());
			ImGui::EndTable();
		}
	}
}

void UDebugImGuiComponent::DrawEnemyDetectionInfo()
{
	if (!EnemyDetectorComponent)
	{
		return;
	}

	if (ImGui::CollapsingHeader("[ 주변 적 감지 ] (Enemy Detection)", ImGuiTreeNodeFlags_DefaultOpen))
	{
		const TArray<TWeakObjectPtr<ABaseEnemyCharacter>>& CachedEnemies = EnemyDetectorComponent->GetCachedEnemies();

		if (CachedEnemies.IsEmpty())
		{
			ImGui::TextDisabled("  현재 감지된 적이 없습니다.");
			return;
		}

		// 1. TMap을 활용하여 클래스별로 적의 수를 카운트합니다.
		TMap<FString, int32> EnemyClassCountMap;
		int32 TotalValidEnemies = 0;

		for (const TWeakObjectPtr<ABaseEnemyCharacter>& WeakEnemy : CachedEnemies)
		{
			if (ABaseEnemyCharacter* Enemy = WeakEnemy.Get())
			{
				// 적의 클래스 이름을 문자열로 추출 (BP_Zombie, BP_Skeleton 등)
				FString ClassName = Enemy->GetClass()->GetName();
				EnemyClassCountMap.FindOrAdd(ClassName, 0)++;
				TotalValidEnemies++;
			}
		}

		ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "총 감지된 적: %d 마리", TotalValidEnemies);
		ImGui::Spacing();

		// 2. 카운트된 정보를 테이블 형태로 깔끔하게 렌더링
		if (ImGui::BeginTable("EnemyDetectionTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
		{
			ImGui::TableSetupColumn("적 클래스명 (Enemy Class)", ImGuiTableColumnFlags_WidthStretch, 0.7f);
			ImGui::TableSetupColumn("개수 (Count)", ImGuiTableColumnFlags_WidthStretch, 0.3f);
			ImGui::TableHeadersRow();

			for (const auto& Pair : EnemyClassCountMap)
			{
				ImGui::TableNextRow();

				// 클래스 명
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%s", TCHAR_TO_UTF8(*Pair.Key));

				// 카운트 수치 (빨간색 강조)
				ImGui::TableSetColumnIndex(1);
				ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%d 마리", Pair.Value);
			}
			ImGui::EndTable();
		}
	}
}

void UDebugImGuiComponent::DrawExperienceInfo()
{
	if (!PlayerExperienceComponent)
		return;

	// 눈에 잘 띄게 약간의 강조 색상을 사용
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.2f, 1.0f));
	bool bExpOpen = ImGui::CollapsingHeader("[ 성장 및 재화 ] (Experience & Wealth)", ImGuiTreeNodeFlags_DefaultOpen);
	ImGui::PopStyleColor();

	if (bExpOpen)
	{
		if (ImGui::BeginTable("ExpTable", 2, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingStretchProp))
		{			
			// 현재 Level (스카이블루)
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::TextDisabled("현재 레벨 (Current Level)");
			ImGui::TableSetColumnIndex(1); ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.0f, 1.0f), "%d", PlayerExperienceComponent->GetCurrentLevel());

			// 현재 XP (스카이블루)
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::TextDisabled("현재 경험치 (Current EXP)");
			ImGui::TableSetColumnIndex(1); ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%.1f", PlayerExperienceComponent->GetCurrentEXP());

			// 현재 골드 (골드)
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::TextDisabled("보유 재화 (Gold)");
			ImGui::TableSetColumnIndex(1); ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "%d G", PlayerExperienceComponent->GetCurrentGold());

			ImGui::EndTable();
		}
	}
}


// 이동 관련 디버그 함수들은 PlayerMovementComponent이 유효할 때만 렌더링하도록 DrawPlayerCharacterInfo()에서 호출

void UDebugImGuiComponent::DrawMovement_Basic()
{
	if (ImGui::TreeNodeEx("기본 정보", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::BeginTable("MovBasicTable", 2, ImGuiTableFlags_BordersInnerH))
		{
			const FVector Vel = PlayerMovementComponent->Velocity;
			ImGuiUtils::DrawRowVector("속도 (Velocity)", Vel);
			ImGuiUtils::DrawRowText("속력 (Speed)", "%.2f", Vel.Size());
			ImGuiUtils::DrawRowText("가속도", "%.2f", PlayerMovementComponent->GetCurrentAcceleration().Size());
			ImGuiUtils::DrawRowBool("이동 중", Vel.Size() > 0.1f);
			ImGui::EndTable();
		}
		ImGui::TreePop();
	}
}

void UDebugImGuiComponent::DrawMovement_State()
{
	if (ImGui::TreeNode("상태"))
	{
		if (ImGui::BeginTable("MovStateTable", 2, ImGuiTableFlags_BordersInnerH))
		{
			ImGuiUtils::DrawRowText("이동 모드", "%d", (int)PlayerMovementComponent->MovementMode);
			ImGuiUtils::DrawRowBool("낙하(점프) 중", PlayerMovementComponent->IsFalling());
			ImGuiUtils::DrawRowBool("비행 중", PlayerMovementComponent->IsFlying());
			ImGuiUtils::DrawRowBool("수영 중", PlayerMovementComponent->IsSwimming());
			ImGuiUtils::DrawRowBool("앉은 상태", PlayerMovementComponent->IsCrouching());
			ImGui::EndTable();
		}
		ImGui::TreePop();
	}
}

void UDebugImGuiComponent::DrawMovement_Speed()
{
	if (ImGui::TreeNode("속도 설정"))
	{
		if (ImGui::BeginTable("MovSpeedTable", 2, ImGuiTableFlags_BordersInnerH))
		{
			ImGuiUtils::DrawRowText("최대 걷기 속도", "%.2f", PlayerMovementComponent->MaxWalkSpeed);
			ImGuiUtils::DrawRowText("최대 가속도", "%.2f", PlayerMovementComponent->MaxAcceleration);
			ImGuiUtils::DrawRowText("멈추는 감속도", "%.2f", PlayerMovementComponent->BrakingDecelerationWalking);
			ImGuiUtils::DrawRowText("지면 마찰력", "%.2f", PlayerMovementComponent->GroundFriction);
			ImGui::EndTable();
		}
		ImGui::TreePop();
	}
}

void UDebugImGuiComponent::DrawMovement_Input()
{
	if (ImGui::TreeNode("입력"))
	{
		if (ImGui::BeginTable("MovInputTable", 2, ImGuiTableFlags_BordersInnerH))
		{
			FVector Input = PlayerMovementComponent->GetLastInputVector();
			ImGuiUtils::DrawRowVector("입력 벡터", Input);
			ImGuiUtils::DrawRowText("입력 크기", "%.2f", Input.Size());
			ImGui::EndTable();
		}
		ImGui::TreePop();
	}
}

void UDebugImGuiComponent::DrawMovement_Physics()
{
	if (ImGui::TreeNode("Physics"))
	{
		if (ImGui::BeginTable("MovPhysicsTable", 2, ImGuiTableFlags_BordersInnerH))
		{
			ImGuiUtils::DrawRowText("질량", "%.2f kg", PlayerMovementComponent->Mass);
			ImGuiUtils::DrawRowText("중력 스케일", "%.2f", PlayerMovementComponent->GravityScale);
			ImGuiUtils::DrawRowBool("물리 상호작용 여부", PlayerMovementComponent->bEnablePhysicsInteraction);
			ImGui::EndTable();
		}
		ImGui::TreePop();
	}
}


void UDebugImGuiComponent::DrawSystemAndMemoryInfo()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// ========================================================
	// 1. 이펙트 서브시스템 (Effect Subsystem) Memory
	// ========================================================
	ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.4f, 0.6f, 1.0f));
	bool bEffectHeaderOpen = ImGui::CollapsingHeader("이펙트 서브시스템 (Effect Subsystem) Memory", ImGuiTreeNodeFlags_DefaultOpen);
	ImGui::PopStyleColor();

	if (bEffectHeaderOpen)
	{
		UEffectSubsystem* EffectSubsys = World->GetSubsystem<UEffectSubsystem>();
		if (EffectSubsys)
		{
			const TMap<FPrimaryAssetId, FLoadedAsset>& AssetMap = EffectSubsys->GetPreloadAssetCounts();

			ImGui::Text("추적 중인 고유 에셋 종류: %d 개", AssetMap.Num());
			ImGui::Spacing();

			if (AssetMap.Num() > 0)
			{
				if (ImGui::BeginTable("EffectMemoryTable", 3, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
				{
					ImGui::TableSetupColumn("에셋 타입 (Type)");
					ImGui::TableSetupColumn("에셋 이름 (Name)");
					ImGui::TableSetupColumn("상태 (Ref Count)");
					ImGui::TableHeadersRow();

					for (const auto& Pair : AssetMap)
					{
						FString AssetType = Pair.Key.PrimaryAssetType.ToString();
						FString AssetName = Pair.Key.PrimaryAssetName.ToString();
						int32 RefCount = Pair.Value.Count;

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::TextDisabled("%s", TCHAR_TO_UTF8(*AssetType));
						ImGui::TableSetColumnIndex(1);
						ImGui::Text("%s", TCHAR_TO_UTF8(*AssetName));
						ImGui::TableSetColumnIndex(2);

						if (RefCount > 0)
							ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "%d (Active)", RefCount);
						else
							ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.3f, 1.0f), "%d (Will GC)", RefCount);
					}
					ImGui::EndTable();
				}
			}
			else
			{
				ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "현재 메모리에 로드(Preload)된 이펙트 에셋이 없습니다.");
			}
		}
	}

	ImGui::Spacing();

	// ========================================================
	// 2. 오브젝트 풀 서브시스템 (Object Pool Subsystem)
	// ========================================================
	ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.6f, 0.4f, 0.2f, 1.0f));
	// 🌟 이 헤더가 접혀있을 때는 내부의 루프와 연산이 아예 실행되지 않습니다! (지연 평가)
	bool bPoolHeaderOpen = ImGui::CollapsingHeader("오브젝트 풀 상태 (Object Pool Stats)", ImGuiTreeNodeFlags_DefaultOpen);
	ImGui::PopStyleColor();

	if (bPoolHeaderOpen)
	{
		DrawObjectPoolInfo();
	}
}

void UDebugImGuiComponent::DrawObjectPoolInfo()
{
	UWorld* World = GetWorld();
	UObjectPoolSubsystem* Pool = IsValid(World) ? World->GetSubsystem<UObjectPoolSubsystem>() : nullptr;
	if (!IsValid(Pool))
	{
		ImGui::TextDisabled("ObjectPoolSubsystem을 사용할 수 없습니다.");
		return;
	}

	const auto& Pools = Pool->GetActorPools();
	ImGui::Text("등록 클래스 항목: %d", Pools.Num());
	ImGui::TextWrapped("등록 = 활성 + 대기 + 무효. 활성/대기는 유효 액터의 Hidden 상태 기준입니다. 실제 공격 중 여부는 아닙니다.");
	ImGui::TextDisabled("클래스를 펼치면 등록된 액터를 확인합니다. 수치는 현재 풀 목록 기준입니다.");
	if (Pools.IsEmpty())
	{
		ImGui::TextDisabled("현재 등록된 풀이 없습니다.");
		return;
	}
	if (!ImGui::BeginTable("ObjectPoolTable", 5, ImGuiTableFlags_BordersInnerH
		| ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp)) return;
	ImGui::TableSetupColumn("클래스 / 액터", ImGuiTableColumnFlags_WidthStretch, 3.f);
	ImGui::TableSetupColumn("등록");
	ImGui::TableSetupColumn("활성");
	ImGui::TableSetupColumn("대기");
	ImGui::TableSetupColumn("무효");
	ImGui::TableHeadersRow();
	int64 TotalEntries = 0, TotalActive = 0, TotalInactive = 0, TotalInvalid = 0;
	for (const auto& Pair : Pools)
	{
		UClass* Class = Pair.Key.Get();
		int32 Active = 0, Inactive = 0, Invalid = 0;
		// Observe the public container; GetPoolStats counts null entries as active.
		for (const AActor* Actor : Pair.Value)
		{
			if (!IsValid(Actor) || Actor->IsActorBeingDestroyed()) ++Invalid;
			else if (Actor->IsHidden()) ++Inactive;
			else ++Active;
		}
		TotalEntries += Pair.Value.Num();
		TotalActive += Active;
		TotalInactive += Inactive;
		TotalInvalid += Invalid;
		ImGui::PushID(Class);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		const bool bOpen = ImGui::TreeNodeEx("PoolClass", ImGuiTreeNodeFlags_None, "%s",
			IsValid(Class) ? TCHAR_TO_UTF8(*Class->GetName()) : "유효하지 않은 클래스");
		if (IsValid(Class) && ImGui::IsItemHovered()) ImGui::SetTooltip("%s", TCHAR_TO_UTF8(*Class->GetPathName()));
		ImGui::TableSetColumnIndex(1); ImGui::Text("%d", Pair.Value.Num());
		ImGui::TableSetColumnIndex(2); ImGui::Text("%d", Active);
		ImGui::TableSetColumnIndex(3); ImGui::TextDisabled("%d", Inactive);
		ImGui::TableSetColumnIndex(4);
		if (Invalid > 0) ImGui::TextColored(ImVec4(1.f, 0.35f, 0.3f, 1.f), "%d", Invalid);
		else ImGui::TextDisabled("0");
		if (bOpen)
		{
			ImGui::TableSetColumnIndex(0);
			for (int32 Index = 0; Index < Pair.Value.Num(); ++Index)
			{
				const AActor* Actor = Pair.Value[Index];
				if (!IsValid(Actor) || Actor->IsActorBeingDestroyed())
				{
					ImGui::TextDisabled("[%d] 무효 / 파괴 중", Index);
					continue;
				}
				ImGui::Text("[%d] %s (%s)", Index, TCHAR_TO_UTF8(*Actor->GetName()), Actor->IsHidden() ? "대기" : "활성");
				if (ImGui::IsItemHovered())
				{
					const AActor* Owner = Actor->GetOwner();
					ImGui::SetTooltip("%s\nOwner: %s\nCollision: %s / Tick: %s",
						TCHAR_TO_UTF8(*Actor->GetPathName()), IsValid(Owner) ? TCHAR_TO_UTF8(*Owner->GetName()) : "없음",
						Actor->GetActorEnableCollision() ? "On" : "Off", Actor->IsActorTickEnabled() ? "On" : "Off");
				}
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0); ImGui::Text("합계 (유효 %lld)", TotalActive + TotalInactive);
	ImGui::TableSetColumnIndex(1); ImGui::Text("%lld", TotalEntries);
	ImGui::TableSetColumnIndex(2); ImGui::Text("%lld", TotalActive);
	ImGui::TableSetColumnIndex(3); ImGui::Text("%lld", TotalInactive);
	ImGui::TableSetColumnIndex(4); ImGui::Text("%lld", TotalInvalid);
	ImGui::EndTable();
}

namespace PetDashboard
{
	constexpr ImGuiTableFlags TableFlags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH
		| ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp;

	bool IsUsable(const APetCompanionCharacter* Pet)
	{
		return IsValid(Pet) && !Pet->IsActorBeingDestroyed();
	}

	FString StatName(EPetStatType Type)
	{
		return StaticEnum<EPetStatType>()->GetDisplayNameTextByValue(static_cast<int64>(Type)).ToString();
	}

	void DrawModifier(const FStatModifier& Modifier)
	{
		const FString Name = StatName(Modifier.StatType);
		switch (Modifier.ModType)
		{
		case EStatModifierType::Additive:
			ImGui::Text("%s: 고정 %+.3f", TCHAR_TO_UTF8(*Name), Modifier.Value);
			break;
		case EStatModifierType::Multiplicative:
			ImGui::Text("%s: 곱연산 %+.1f%%", TCHAR_TO_UTF8(*Name), Modifier.Value * 100.f);
			break;
		case EStatModifierType::Override:
			ImGui::Text("%s: 덮어쓰기 %.3f", TCHAR_TO_UTF8(*Name), Modifier.Value);
			break;
		default:
			ImGui::TextDisabled("알 수 없는 연산: %d", static_cast<int32>(Modifier.ModType));
			break;
		}
	}
}

void UDebugImGuiComponent::DrawPetInfo()
{
	AActor* Owner = GetOwner();
	UPetManagerComponent* Manager = IsValid(Owner) && IsValid(GetWorld())
		? Owner->FindComponentByClass<UPetManagerComponent>() : nullptr;
	if (!IsValid(Manager))
	{
		InspectedPet.Reset();
		ImGui::TextDisabled("펫 관리 컴포넌트를 사용할 수 없습니다.");
		return;
	}
	const auto& Pets = Manager->GetPetList();
	APawn* Pawn = Cast<APawn>(Owner);
	ACombatPlayerState* PlayerState = IsValid(Pawn) ? Pawn->GetPlayerState<ACombatPlayerState>() : nullptr;
	if (!PetDashboard::IsUsable(InspectedPet.Get()) || !Pets.Contains(InspectedPet.Get()))
	{
		InspectedPet.Reset();
		PetActionResult.Reset();
		for (const auto& Entry : Pets)
		{
			if (PetDashboard::IsUsable(Entry.Get()))
			{
				InspectedPet = Entry.Get();
				break;
			}
		}
	}
	int32 ValidCount = 0;
	for (const auto& Entry : Pets) ValidCount += PetDashboard::IsUsable(Entry.Get()) ? 1 : 0;
	ImGui::Text("보유 펫 %d", ValidCount);
	ImGui::SameLine();
	ImGui::TextDisabled("행을 선택하면 아래에 상세 정보가 표시됩니다.");
	if (ImGui::BeginTable("PetOverview", 4, PetDashboard::TableFlags))
	{
		ImGui::TableSetupColumn("펫", ImGuiTableColumnFlags_WidthStretch, 2);
		ImGui::TableSetupColumn("체력");
		ImGui::TableSetupColumn("아이템");
		ImGui::TableSetupColumn("기록 피해");
		ImGui::TableHeadersRow();
		for (const auto& Entry : Pets)
		{
			APetCompanionCharacter* Pet = Entry.Get();
			if (!PetDashboard::IsUsable(Pet)) continue;
			ImGui::PushID(Pet);
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			if (ImGui::Selectable(TCHAR_TO_UTF8(*Pet->GetName()), InspectedPet.Get() == Pet,
				ImGuiSelectableFlags_SpanAllColumns))
			{
				InspectedPet = Pet;
				PetActionResult.Reset();
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", TCHAR_TO_UTF8(*Pet->GetPathName()));
			UPetStatComponent* Stats = Pet->GetStatComponent();
			UPetItemComponent* Items = Pet->GetItemComponent();
			ImGui::TableSetColumnIndex(1);
			// GetFinalStat internally reads the item component, so both must exist.
			if (IsValid(Stats) && IsValid(Items))
			{
				ImGui::Text("%.0f / %.0f", Stats->GetFinalStat(EPetStatType::CurrentHealth),
					Stats->GetFinalStat(EPetStatType::MaxHealth));
			}
			else ImGui::TextDisabled("확인 불가");
			ImGui::TableSetColumnIndex(2);
			if (IsValid(Items)) ImGui::Text("%d", Items->GetOwnedItems().Num());
			else ImGui::TextDisabled("없음");
			ImGui::TableSetColumnIndex(3);
			if (IsValid(PlayerState)) ImGui::Text("%.1f", PlayerState->GetPetDamageStats().FindRef(Pet->GetFName()));
			else ImGui::TextDisabled("대기 중");
			ImGui::PopID();
		}
		ImGui::EndTable();
	}
	if (ImGui::CollapsingHeader("전체 피해 기록 (이전 펫 포함)"))
	{
		ImGui::TextDisabled("PlayerState 누적 기록입니다. 실시간 DPS가 아닙니다.");
		if (!IsValid(PlayerState)) ImGui::TextDisabled("PlayerState를 기다리는 중입니다.");
		else if (PlayerState->GetPetDamageStats().IsEmpty()) ImGui::TextDisabled("기록이 없습니다.");
		else if (ImGui::BeginTable("PetDamageHistory", 2, PetDashboard::TableFlags))
		{
			ImGui::TableSetupColumn("펫 ID");
			ImGui::TableSetupColumn("기록 피해");
			ImGui::TableHeadersRow();
			for (const auto& Pair : PlayerState->GetPetDamageStats())
				ImGuiUtils::DrawRowText(TCHAR_TO_UTF8(*Pair.Key.ToString()), "%.1f", Pair.Value);
			ImGui::EndTable();
		}
	}
	APetCompanionCharacter* Pet = InspectedPet.Get();
	if (!PetDashboard::IsUsable(Pet))
	{
		ImGui::TextDisabled("소유 중인 펫이 없습니다.");
		return;
	}
	ImGui::Separator();
	ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.f, 1.f), "%s", TCHAR_TO_UTF8(*Pet->GetName()));
	if (!PetActionResult.IsEmpty()) ImGui::TextWrapped("%s", TCHAR_TO_UTF8(*PetActionResult));
	FGuid RemoveId;
	ImGui::PushID(Pet);
	if (ImGui::BeginTabBar("PetDetails"))
	{
		if (ImGui::BeginTabItem("상태"))
		{
			DrawPetStatus(Pet);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("아이템"))
		{
			RemoveId = DrawPetInventory(Pet);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("공격"))
		{
			DrawPetCombat(Pet);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("시너지"))
		{
			DrawPetSynergies(Pet->GetItemComponent());
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::PopID();
	// No pet/item array is being iterated when production delegates execute.
	if (RemoveId.IsValid() && IsValid(Manager) && PetDashboard::IsUsable(Pet)
		&& Manager->GetPetList().Contains(Pet) && IsValid(Pet->GetItemComponent())
		&& IsValid(Pet->GetStatComponent()))
	{
		TWeakObjectPtr<UPetItemComponent> Items = Pet->GetItemComponent();
		Manager->RemoveItemFromPet(Pet, RemoveId);
		const bool bRemoved = Items.IsValid() && !Items->GetOwnedItems().ContainsByPredicate(
			[RemoveId](const FPetItemInstance& Item) { return Item.InstanceId == RemoveId; });
		PetActionResult = bRemoved ? TEXT("선택한 인스턴스 제거를 확인했습니다.") : TEXT("제거 결과를 확인하지 못했습니다.");
	}
}

void UDebugImGuiComponent::DrawPetStatus(APetCompanionCharacter* Pet)
{
	UPetStatComponent* Stats = Pet->GetStatComponent();
	if (IsValid(Stats) && IsValid(Pet->GetItemComponent()))
	{
		const float Health = Stats->GetFinalStat(EPetStatType::CurrentHealth);
		const float MaxHealth = Stats->GetFinalStat(EPetStatType::MaxHealth);
		const FString Overlay = FString::Printf(TEXT("체력 %.1f / %.1f"), Health, MaxHealth);
		ImGui::ProgressBar(MaxHealth > 0.f ? FMath::Clamp(Health / MaxHealth, 0.f, 1.f) : 0.f,
			ImVec2(-1, 0), TCHAR_TO_UTF8(*Overlay));
		if (ImGui::BeginTable("FinalStats", 2, PetDashboard::TableFlags))
		{
			ImGui::TableSetupColumn("스탯");
			ImGui::TableSetupColumn("현재 최종값");
			ImGui::TableHeadersRow();
			const EPetStatType Types[] = { EPetStatType::AttackPower, EPetStatType::Defense,
				EPetStatType::AttackSpeed, EPetStatType::MoveSpeed, EPetStatType::CriticalRate,
				EPetStatType::CriticalDamage, EPetStatType::DetectionRange, EPetStatType::AttackRange,
				EPetStatType::Luck, EPetStatType::FireDamageBonus, EPetStatType::IceDamageBonus,
				EPetStatType::WindDamageBonus, EPetStatType::PoisonDamageBonus, EPetStatType::BleedDamageBonus };
			for (EPetStatType Type : Types)
			{
				const float Value = Stats->GetFinalStat(Type);
				const bool bPercent = Type == EPetStatType::CriticalRate || Type == EPetStatType::FireDamageBonus
					|| Type == EPetStatType::IceDamageBonus || Type == EPetStatType::WindDamageBonus
					|| Type == EPetStatType::PoisonDamageBonus || Type == EPetStatType::BleedDamageBonus;
				const char* Format = bPercent ? "%.1f%%" : Type == EPetStatType::CriticalDamage ? "%.2f 배"
					: Type == EPetStatType::AttackSpeed ? "%.2f 회/초" : "%.2f";
				ImGuiUtils::DrawRowText(TCHAR_TO_UTF8(*PetDashboard::StatName(Type)), Format, bPercent ? Value * 100.f : Value);
			}
			ImGui::EndTable();
		}
	}
	else ImGui::TextDisabled("스탯 또는 아이템 컴포넌트가 없어 최종값을 확인할 수 없습니다.");
	if (ImGui::CollapsingHeader("위치 / AI / 컴포넌트"))
	{
		if (ImGui::BeginTable("PetActor", 2, PetDashboard::TableFlags))
		{
			const FVector Location = Pet->GetActorLocation();
			ImGuiUtils::DrawRowText("위치", "%.1f, %.1f, %.1f", Location.X, Location.Y, Location.Z);
			ImGuiUtils::DrawRowText("AI 컨트롤러", "%s", IsValid(Pet->GetController()) ? TCHAR_TO_UTF8(*Pet->GetController()->GetName()) : "없음");
			ImGuiUtils::DrawRowBool("StatComponent", IsValid(Stats));
			ImGuiUtils::DrawRowBool("ItemComponent", IsValid(Pet->GetItemComponent()));
			ImGuiUtils::DrawRowBool("CombatComponent", IsValid(Pet->GetCombatComponent()));
			ImGui::EndTable();
		}
	}
}

FGuid UDebugImGuiComponent::DrawPetInventory(APetCompanionCharacter* Pet)
{
	FGuid RemoveId;
	UPetItemComponent* Items = Pet->GetItemComponent();
	if (!IsValid(Items))
	{
		ImGui::TextDisabled("아이템 컴포넌트가 없습니다.");
		return RemoveId;
	}
	ImGui::Text("보유 %d개", Items->GetOwnedItems().Num());
	ImGui::TextDisabled("아이템을 펼치면 태그와 적용 Modifier를 확인할 수 있습니다.");
	if (ImGui::BeginTable("Inventory", 3, PetDashboard::TableFlags))
	{
		ImGui::TableSetupColumn("아이템 / 효과", ImGuiTableColumnFlags_WidthStretch, 3);
		ImGui::TableSetupColumn("인스턴스", ImGuiTableColumnFlags_WidthStretch, 1);
		ImGui::TableSetupColumn("액션", ImGuiTableColumnFlags_WidthFixed, 65);
		ImGui::TableHeadersRow();
		for (const FPetItemInstance& Item : Items->GetOwnedItems())
		{
			const FString Id = Item.InstanceId.ToString();
			ImGui::PushID(TCHAR_TO_UTF8(*Id));
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			const bool bHasAsset = IsValid(Item.LoadedData.Get());
			const FString Name = TEXT("이름 미제공 (DataAsset 없음)");//bHasAsset ? Item.LoadedData->ItemName.ToString() : TEXT("이름 미제공 (DataAsset 없음)");
			const bool bOpen = ImGui::TreeNodeEx("Details", ImGuiTreeNodeFlags_SpanAvailWidth,
				"%s", TCHAR_TO_UTF8(*Name));
			if (bOpen)
			{
				if (bHasAsset)
				{
					ImGui::TextWrapped("태그: %s", TCHAR_TO_UTF8(*Item.LoadedData->SynergyTags.ToStringSimple()));
				}
				else ImGui::TextWrapped("현재 스탯 아이템은 원본 이름/행 정보가 저장되지 않습니다.");
				bool bFound = false;
				for (const FStatModifier& Modifier : Items->GetStatModifiers())
				{
					if (Modifier.SourceId == Item.InstanceId)
					{
						PetDashboard::DrawModifier(Modifier);
						bFound = true;
					}
				}
				if (!bFound) ImGui::TextDisabled("이 인스턴스의 스탯 Modifier 없음");
				ImGui::TreePop();
			}
			ImGui::TableSetColumnIndex(1);
			ImGui::TextDisabled("%s", TCHAR_TO_UTF8(*Id.Left(8)));
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", TCHAR_TO_UTF8(*Id));
			ImGui::TableSetColumnIndex(2);
			// A non-null but invalid asset must not be handed to RemoveItem's raw pointer path.
			ImGui::BeginDisabled(!Item.InstanceId.IsValid() || !IsValid(Pet->GetStatComponent())
				|| (Item.LoadedData.Get() != nullptr && !bHasAsset));
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.55f, 0.2f, 0.2f, 1.f));
			if (ImGui::SmallButton("제거")) RemoveId = Item.InstanceId;
			ImGui::PopStyleColor();
			ImGui::EndDisabled();
			ImGui::PopID();
		}
		ImGui::EndTable();
	}
	if (Items->GetOwnedItems().IsEmpty()) ImGui::TextDisabled("보유 아이템이 없습니다.");
	return RemoveId;
}

void UDebugImGuiComponent::DrawPetCombat(APetCompanionCharacter* Pet)
{
	if (!PetDashboard::IsUsable(Pet)) return;
	UPetCombatComponent* Combat = Pet->GetCombatComponent();
	if (!IsValid(Combat))
	{
		ImGui::TextDisabled("전투 컴포넌트가 없습니다.");
		return;
	}
	const AActor* Target = Combat->GetCurrentTarget();
	const ABaseWeapon* Weapon = Combat->DefaultWeaponClassTest.Get();
	const bool bHasWeapon = IsValid(Weapon) && !Weapon->IsActorBeingDestroyed();
	ImGui::TextDisabled("현재 무기: PetCombatComponent가 공격에 사용하는 인스턴스");
	if (ImGui::BeginTable("CombatRuntime", 2, PetDashboard::TableFlags))
	{
		ImGuiUtils::DrawRowText("현재 타겟", "%s", IsValid(Target) && !Target->IsActorBeingDestroyed()
			? TCHAR_TO_UTF8(*Target->GetName()) : "없음");
		ImGuiUtils::DrawRowText("실제 무기 (DefaultWeaponClassTest)", "%s", bHasWeapon ? TCHAR_TO_UTF8(*Weapon->GetName()) : "없음 / 파괴 중");
		if (bHasWeapon)
		{
			ImGuiUtils::DrawRowText("실제 클래스", "%s", TCHAR_TO_UTF8(*Weapon->GetClass()->GetPathName()));
			ImGuiUtils::DrawRowText("무기 인스턴스 경로", "%s", TCHAR_TO_UTF8(*Weapon->GetPathName()));
			const AActor* Owner = Weapon->GetOwner();
			const APawn* Instigator = Weapon->GetInstigator();
			const AActor* AttachParent = Weapon->GetAttachParentActor();
			ImGuiUtils::DrawRowText("Owner", "%s", IsValid(Owner) ? TCHAR_TO_UTF8(*Owner->GetName()) : "없음");
			ImGuiUtils::DrawRowBool("Owner가 선택 펫", Owner == Pet);
			ImGuiUtils::DrawRowText("Instigator", "%s", IsValid(Instigator) ? TCHAR_TO_UTF8(*Instigator->GetName()) : "없음");
			ImGuiUtils::DrawRowText("부착 부모", "%s", IsValid(AttachParent) ? TCHAR_TO_UTF8(*AttachParent->GetName()) : "없음");
			ImGuiUtils::DrawRowBool("발사체 무기 (AFireWeapon)", Weapon->IsA<AFireWeapon>());
			ImGuiUtils::DrawRowBool("Hidden", Weapon->IsHidden());
			ImGuiUtils::DrawRowVector("무기 위치", Weapon->GetActorLocation());
		}
		ImGui::EndTable();
	}
	if (bHasWeapon && Weapon->IsA<AFireWeapon>())
	{
		ImGui::TextWrapped("발사체 클래스 / 적용 수·속도·크기: AFireWeapon의 공개 조회 API가 필요합니다. 전체 클래스별 풀 수치는 시스템 탭에서 확인할 수 있습니다.");
	}
	ImGui::TextDisabled("공격 타이머 상태와 최종 공격 결과는 공개 조회 API가 없어 표시하지 않습니다.");
}

void UDebugImGuiComponent::DrawPetSynergies(UPetItemComponent* Items)
{
	if (!IsValid(Items))
	{
		ImGui::TextDisabled("아이템 컴포넌트가 없습니다.");
		return;
	}
	ImGui::Text("활성 시너지 %d개", Items->GetActiveSynergies().Num());
	if (Items->GetActiveSynergies().IsEmpty()) ImGui::TextDisabled("활성 시너지가 없습니다.");
	for (const auto& Entry : Items->GetActiveSynergies())
	{
		const UPetSynergyDataAsset* Synergy = Entry.Get();
		if (!IsValid(Synergy))
		{
			ImGui::TextDisabled("유효하지 않은 시너지 참조");
			continue;
		}
		ImGui::PushID(Synergy);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.85f, 0.5f, 1.f));
		const bool bOpen = ImGui::TreeNodeEx("Synergy", ImGuiTreeNodeFlags_None,
			"%s", TCHAR_TO_UTF8(*Synergy->SynergyName.ToString()));
		ImGui::PopStyleColor();
		if (bOpen)
		{
			ImGui::TextDisabled("요구 조건 (설정값)");
			for (const auto& Pair : Synergy->RequiredTagCounts)
				ImGui::BulletText("%s: %d개", TCHAR_TO_UTF8(*Pair.Key.ToString()), Pair.Value);
			ImGui::TextDisabled("스탯 보너스 (설정값)");
			for (const FStatModifier& Modifier : Synergy->StatBonuses) PetDashboard::DrawModifier(Modifier);
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
}
