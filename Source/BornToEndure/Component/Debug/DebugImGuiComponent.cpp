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

#include "Item/Weapon/RifleWeapon.h"
#include "Item/Projectile/BaseProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

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
#include "Data/PetProjectileItemDataAsset.h"
#include "Stat/PetStatTypes.h"
#include "Component/PlayerExperienceComponent.h"
#include "PlayerState/CombatPlayerState.h"

#include "Component/EnemyDetectorComponent.h" 
#include "Character/Enemy/BaseEnemyCharacter.h" 

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
	LoadKoreanFontToImGui();

	AActor* Owner = GetOwner();

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
	PlayerExperienceComponent = Owner->FindComponentByClass<UPlayerExperienceComponent>();
}

void UDebugImGuiComponent::BeginPlay()
{
	Super::BeginPlay();

	if (FImGuiModule::IsAvailable())
	{
		FImGuiDelegate Delegate = FImGuiDelegate::CreateUObject(this, &UDebugImGuiComponent::DebugDrawPlayerInfo);
		ImGuiDelegateHandle = FImGuiModule::Get().AddWorldImGuiDelegate(Delegate);
		UE_LOG(LogTemp, Log, TEXT("ImGui Delegate subscribed!"));
	}
}

void UDebugImGuiComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (FImGuiModule::IsAvailable() && ImGuiDelegateHandle.IsValid())
	{
		FImGuiModule::Get().RemoveImGuiDelegate(ImGuiDelegateHandle);
		ImGuiDelegateHandle.Reset();
	}

	Super::EndPlay(EndPlayReason);
}

void UDebugImGuiComponent::LoadKoreanFontToImGui()
{
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
	ImGui::SetNextWindowBgAlpha(0.85f);
	ImGui::SetNextWindowSize(ImVec2(500, 700), ImGuiCond_FirstUseEver);

	ImGui::Begin("종합 디버그 패널 (System Debug Panel)");

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
		//DrawWeaponInfo();
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
		if (ImGui::BeginTabItem("월드 & 메모리 (World & Memory)"))
		{
			DrawSystemAndMemoryInfo();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("펫 & 동료 (Pets)"))
		{
			DrawPetInfo();
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

void UDebugImGuiComponent::DrawWeaponInfo()
{
	ABaseWeapon* EquippedWeapon = nullptr;
	if (PlayerCharacter)
	{
		PlayerCharacter->GetWeaponBase(EquippedWeapon);
	}

	if (!EquippedWeapon)
	{
		if (ImGui::CollapsingHeader("무기 정보 (Weapon Info)", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "현재 장착된 무기가 없습니다.");
		}
		return;
	}

	if (ImGui::CollapsingHeader("무기 정보 (Weapon Info)", ImGuiTreeNodeFlags_DefaultOpen))
	{
		// ==============================================
		// 1. 기본 무기 상태 (Basic Info)
		// ==============================================
		if (ImGui::TreeNodeEx("1. 기본 상태 (Basic Info)", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::BeginTable("BasicInfoTable", 2, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingFixedFit))
			{
				ImGuiUtils::DrawRowText("무기 종류", "%s", TCHAR_TO_UTF8(*UEnum::GetValueAsString(EquippedWeapon->WeaponType)));

				AActor* InstigatorActor = EquippedWeapon->GetInstigator();
				ImGuiUtils::DrawRowText("소유자 (Instigator)", "%s", InstigatorActor ? TCHAR_TO_UTF8(*InstigatorActor->GetName()) : "없음 (None)");

				if (UStaticMeshComponent* WeaponMesh = EquippedWeapon->GetWeaponStaticMesh())
				{
					ImGuiUtils::DrawRowBool("물리 시뮬레이션 활성화", WeaponMesh->IsSimulatingPhysics());

					FTransform LHIKTransform = WeaponMesh->GetSocketTransform(FName("LHIK"), ERelativeTransformSpace::RTS_World);
					ImGuiUtils::DrawRowVector("왼손 IK 소켓", LHIKTransform.GetLocation());

					FTransform MuzzleTransform = WeaponMesh->GetSocketTransform(FName("Muzzle"), ERelativeTransformSpace::RTS_World);
					ImGuiUtils::DrawRowVector("총구(Muzzle) 위치", MuzzleTransform.GetLocation());
				}
				ImGui::EndTable();
			}
			ImGui::TreePop();
		}

		// ==============================================
		// 2. 이펙트 데이터 (Effect Data)
		// ==============================================
		if (ImGui::TreeNodeEx("2. 이펙트 데이터 (Effect Data)", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::BeginTable("EffectDataTable", 2, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingFixedFit))
			{
				ImGuiUtils::DrawRowText("공격 사운드 에셋", "%s", EquippedWeapon->AttackSoundId.IsValid() ? TCHAR_TO_UTF8(*EquippedWeapon->AttackSoundId.ToString()) : "지정되지 않음 (None)");
				ImGuiUtils::DrawRowText("공격 나이아가라 에셋", "%s", EquippedWeapon->AttackNiagaraId.IsValid() ? TCHAR_TO_UTF8(*EquippedWeapon->AttackNiagaraId.ToString()) : "지정되지 않음 (None)");
				ImGui::EndTable();
			}
			ImGui::TreePop();
		}

		// ==============================================
		// 3. 오브젝트 풀링 및 발사체 스펙 (Projectile Info)
		// ==============================================
		if (ImGui::TreeNodeEx("3. 발사체 및 풀링 정보 (Projectile Pool)", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::BeginTable("ProjectileTable", 2, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingFixedFit))
			{
				ImGuiUtils::DrawRowText("요구 풀 크기 (PoolSize)", "%d", EquippedWeapon->PoolSize);

				if (ARifleWeapon* Rifle = Cast<ARifleWeapon>(EquippedWeapon))
				{
					TSubclassOf<ABaseProjectile> ProjClass = Rifle->ProjectileClass;
					if (ProjClass)
					{
						ImGuiUtils::DrawRowText("발사체 클래스", "%s", TCHAR_TO_UTF8(*ProjClass->GetName()));

						// [추가] CDO를 통한 발사체 스펙 세부 분석
						if (ABaseProjectile* ProjCDO = ProjClass->GetDefaultObject<ABaseProjectile>())
						{
							// 기본 스탯 (BaseProjectile 멤버)
							ImGuiUtils::DrawRowText(" └ 데미지 (Damage)", "%.1f", ProjCDO->ProjectileDamage);
							ImGuiUtils::DrawRowText(" └ 수명 (Lifespan)", "%.1f sec", ProjCDO->ProjectileLifespan);
							ImGuiUtils::DrawRowText(" └ 데미지 타입 (DamageType)", "%s", ProjCDO->DamageType ? TCHAR_TO_UTF8(*ProjCDO->DamageType->GetName()) : "None");

							// 콜리전 컴포넌트 정보
							USphereComponent* SphereComp = nullptr;
							ProjCDO->GetSphereComponent(SphereComp);
							if (SphereComp)
							{
								ImGuiUtils::DrawRowText(" └ 충돌 반경 (Radius)", "%.1f", SphereComp->GetScaledSphereRadius());
								ImGuiUtils::DrawRowText(" └ 충돌 프로필", "%s", TCHAR_TO_UTF8(*SphereComp->GetCollisionProfileName().ToString()));
							}

							// 무브먼트 컴포넌트 정보
							UProjectileMovementComponent* MoveComp = nullptr;
							ProjCDO->GetProjectileMovementComponent(MoveComp);
							if (MoveComp)
							{
								ImGuiUtils::DrawRowText(" └ 초기 속도 (Initial Speed)", "%.1f", MoveComp->InitialSpeed);
								ImGuiUtils::DrawRowText(" └ 최대 속도 (Max Speed)", "%.1f", MoveComp->MaxSpeed);
								ImGuiUtils::DrawRowText(" └ 중력 스케일 (Gravity)", "%.2f", MoveComp->ProjectileGravityScale);
								ImGuiUtils::DrawRowBool(" └ 도탄 여부 (Should Bounce)", MoveComp->bShouldBounce);
							}
						}

						// 풀링 통계 정보
						if (UWorld* World = GetWorld())
						{
							if (UObjectPoolSubsystem* PoolSubsystem = World->GetSubsystem<UObjectPoolSubsystem>())
							{
								int32 TotalPool = 0, ActiveCount = 0, InactiveCount = 0;
								PoolSubsystem->GetPoolStats(ProjClass, TotalPool, ActiveCount, InactiveCount);

								ImGuiUtils::DrawRowText("풀 통계 - 생성량 (Total)", "%d", TotalPool);
								ImGuiUtils::DrawRowText("풀 통계 - 활성 (Active)", "%d", ActiveCount);
								ImGuiUtils::DrawRowText("풀 통계 - 대기 (Inactive)", "%d", InactiveCount);
							}
						}
					}
					else
					{
						ImGuiUtils::DrawRowText("발사체 상태", "클래스가 BP에 지정되지 않음 (Null)");
					}
				}
				else
				{
					ImGuiUtils::DrawRowText("발사체 상태", "원거리 타격 무기가 아님 (Not Rifle)");
				}

				ImGui::EndTable();
			}
			ImGui::TreePop();
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
			// 현재 XP (스카이블루)
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::TextDisabled("현재 경험치 (Current XP)");
			ImGui::TableSetColumnIndex(1); ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%.1f", PlayerExperienceComponent->GetCurrentXP());

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
		UObjectPoolSubsystem* PoolSubsys = World->GetSubsystem<UObjectPoolSubsystem>();
		if (PoolSubsys)
		{
			const auto& ActorPools = PoolSubsys->GetActorPools();

			ImGui::Text("관리 중인 클래스 종류: %d 개", ActorPools.Num());
			ImGui::Spacing();

			if (ActorPools.Num() > 0)
			{
				if (ImGui::BeginTable("ObjectPoolTable", 4, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
				{
					ImGui::TableSetupColumn("클래스 (Class)");
					ImGui::TableSetupColumn("전체 (Total)");
					ImGui::TableSetupColumn("활성 (Active)");
					ImGui::TableSetupColumn("대기 (Inactive)");
					ImGui::TableHeadersRow();

					for (const auto& Pair : ActorPools)
					{
						TSubclassOf<AActor> ActorClass = Pair.Key;
						if (!ActorClass) continue;

						// UI가 펼쳐져 있을 때만 순회하여 상태를 카운트합니다.
						int32 Total = 0, Active = 0, Inactive = 0;
						PoolSubsys->GetPoolStats(ActorClass, Total, Active, Inactive);

						ImGui::TableNextRow();

						// 클래스명 출력 (접두어 제거를 위해 GetName() 사용)
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("%s", TCHAR_TO_UTF8(*ActorClass->GetName()));

						// 전체 개수
						ImGui::TableSetColumnIndex(1);
						ImGui::Text("%d", Total);

						// 활성화(날아가는 중) 개수 - 초록색
						ImGui::TableSetColumnIndex(2);
						if (Active > 0) ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "%d", Active);
						else ImGui::TextDisabled("0");

						// 비활성화(풀 대기 중) 개수 - 회색
						ImGui::TableSetColumnIndex(3);
						if (Inactive > 0) ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%d", Inactive);
						else ImGui::TextDisabled("0");
					}
					ImGui::EndTable();
				}
			}
			else
			{
				ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "현재 초기화된 오브젝트 풀이 없습니다.");
			}
		}
	}
}

void UDebugImGuiComponent::DrawPetInfo()
{
	if (!PetManagerComponent)
	{
		ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "[오류] 플레이어에게 PetManagerComponent가 없습니다.");
		return;
	}

	// ---------------------------------------------------------
		// [ 🌟 신규: Pet별 누적 데미지 통계 테이블 ]
		// ---------------------------------------------------------
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	TObjectPtr<ACombatPlayerState> PS = OwningPawn ? OwningPawn->GetPlayerState<ACombatPlayerState>() : nullptr;
	if (PS)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.0f, 0.4f, 1.0f));
		bool bStatsOpen = ImGui::TreeNodeEx("[ 펫별 전투 통계 ] (Total Combat Stats)", ImGuiTreeNodeFlags_DefaultOpen);
		ImGui::PopStyleColor();

		if (bStatsOpen)
		{
			// 통계 데이터 가져오기 (ACombatPlayerState에 GetPetDamageStats()가 있다고 가정)
			const TMap<FName, float>& StatsMap = PS->GetPetDamageStats();

			if (StatsMap.IsEmpty())
			{
				ImGui::TextDisabled("  누적된 전투 데이터가 없습니다.");
			}
			else
			{
				if (ImGui::BeginTable("PetTotalDamageTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
				{
					// 테이블 헤더 설정
					ImGui::TableSetupColumn("펫 이름 (Display Name)", ImGuiTableColumnFlags_WidthStretch, 0.4f);
					ImGui::TableSetupColumn("ID (FName)", ImGuiTableColumnFlags_WidthStretch, 0.3f);
					ImGui::TableSetupColumn("누적 데미지 (Total Damage)", ImGuiTableColumnFlags_WidthStretch, 0.3f);
					ImGui::TableHeadersRow();

					for (const auto& Pair : StatsMap)
					{
						ImGui::TableNextRow();

						// 1. 펫 이름 (현재 활성화된 펫이 있다면 이름을 찾고, 없으면 ID 출력)
						ImGui::TableSetColumnIndex(0);
						FString DisplayName = Pair.Key.ToString();
						// PetManager를 통해 실제 존재하는 펫의 가독성 좋은 이름을 찾으려는 시도
						for (auto& PetPtr : PetManagerComponent->GetOwnedPets())
						{
							if (PetPtr && PetPtr->GetFName() == Pair.Key)
							{
								DisplayName = PetPtr->GetName();
								break;
							}
						}
						ImGui::Text("%s", TCHAR_TO_UTF8(*DisplayName));

						// 2. FName (고유 ID)
						ImGui::TableSetColumnIndex(1);
						ImGui::TextDisabled("%s", TCHAR_TO_UTF8(*Pair.Key.ToString()));

						// 3. 누적 데미지 (강조 색상)
						ImGui::TableSetColumnIndex(2);
						ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%.1f", Pair.Value);
					}
					ImGui::EndTable();
				}
			}
			ImGui::TreePop();
		}
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
	}
	else
	{
		ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "[경고] 플레이어 상태에서 전투 통계 데이터를 찾을 수 없습니다.");
	}

	const TArray<TObjectPtr<APetCompanionCharacter>>& PetList = PetManagerComponent->GetOwnedPets();

	// ---------------------------------------------------------
	// [상단 대시보드 요약]
	// ---------------------------------------------------------
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.9f, 1.0f, 1.0f));
	ImGui::TextWrapped("총 보유 펫: %d 마리", PetList.Num());
	ImGui::PopStyleColor();
	ImGui::Separator();
	ImGui::Spacing();

	if (PetList.IsEmpty())
	{
		ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "소유 중인 펫이 없습니다. (아이템 획득 또는 펫 스폰 대기 중)");
		return;
	}

	// ---------------------------------------------------------
	// [개별 펫 정보 순회]
	// ---------------------------------------------------------
	for (int32 i = 0; i < PetList.Num(); ++i)
	{
		APetCompanionCharacter* Pet = PetList[i].Get();
		if (!Pet) continue;

		ImGui::PushID(i); // 다중 펫 UI 충돌 방지 고유 ID

		ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.15f, 0.5f, 0.35f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.2f, 0.6f, 0.45f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.25f, 0.7f, 0.55f, 1.0f));

		FString PetHeaderName = FString::Printf(TEXT("[ Pet %d ] : %s"), i, *Pet->GetName());
		bool bPetNodeOpen = ImGui::CollapsingHeader(TCHAR_TO_UTF8(*PetHeaderName), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed);

		ImGui::PopStyleColor(3);

		if (bPetNodeOpen)
		{
			ImGui::Indent(15.0f);
			ImGui::Spacing();

			// ==========================================
			// 1. 기본 정보 (이모티콘 제거 및 표준 기호 사용)
			// ==========================================
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
			bool bBasicOpen = ImGui::TreeNodeEx("[ 기본 정보 ] (Actor Basic)", ImGuiTreeNodeFlags_DefaultOpen);
			ImGui::PopStyleColor();
			if (bBasicOpen)
			{
				if (ImGui::BeginTable("PetBasicTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
				{
					ImGuiUtils::DrawRowVector("월드 위치 (Location)", Pet->GetActorLocation());
					ImGuiUtils::DrawRowText("컨트롤러 (AI)", "%s", Pet->GetController() ? TCHAR_TO_UTF8(*Pet->GetController()->GetName()) : "None");
					ImGui::EndTable();
				}
				ImGui::TreePop();
			}
			ImGui::Spacing();

			// ==========================================
			// 2. 스탯 정보 (이모티콘 제거 및 표준 기호 사용)
			// ==========================================
			UPetStatComponent* StatComp = Pet->GetStatComponent();
			if (StatComp)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
				bool bStatOpen = ImGui::TreeNodeEx("[ 핵심 스탯 ] (Status)", ImGuiTreeNodeFlags_DefaultOpen);
				ImGui::PopStyleColor();
				if (bStatOpen)
				{
					if (ImGui::BeginTable("PetStatTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
					{
						ImGuiUtils::DrawRowText("공격력 (AttackPower)", "%.1f", StatComp->GetFinalStat(EPetStatType::AttackPower));
						ImGuiUtils::DrawRowText("체력 (Health)", "%.1f / %.1f", StatComp->GetFinalStat(EPetStatType::CurrentHealth), StatComp->GetFinalStat(EPetStatType::MaxHealth));
						ImGuiUtils::DrawRowText("방어력 (Defense)", "%.1f", StatComp->GetFinalStat(EPetStatType::Defense));
						ImGuiUtils::DrawRowText("공격 속도 (AttackSpeed)", "%.2f 회/초", StatComp->GetFinalStat(EPetStatType::AttackSpeed));
						ImGuiUtils::DrawRowText("이동 속도 (MoveSpeed)", "%.1f", StatComp->GetFinalStat(EPetStatType::MoveSpeed));

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::TextDisabled("치명타 확률 (Crit Rate)");
						ImGui::TableSetColumnIndex(1); ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.2f, 1.0f), "%.1f%%", StatComp->GetFinalStat(EPetStatType::CriticalRate) * 100.f);

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::TextDisabled("치명타 데미지 (Crit Dmg)");
						ImGui::TableSetColumnIndex(1); ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.2f, 1.0f), "%.2f 배", StatComp->GetFinalStat(EPetStatType::CriticalDamage));

						auto DrawElementBonus = [](const char* Name, float Value, ImVec4 Color) {
							if (Value > 0.f) {
								ImGui::TableNextRow();
								ImGui::TableSetColumnIndex(0); ImGui::TextDisabled("%s", Name);
								ImGui::TableSetColumnIndex(1); ImGui::TextColored(Color, "+%.1f%%", Value * 100.f);
							}
							};
						DrawElementBonus("화염 뎀증 (Fire)", StatComp->GetFinalStat(EPetStatType::FireDamageBonus), ImVec4(1.0f, 0.4f, 0.2f, 1.0f));
						DrawElementBonus("냉기 뎀증 (Ice)", StatComp->GetFinalStat(EPetStatType::IceDamageBonus), ImVec4(0.2f, 0.8f, 1.0f, 1.0f));
						DrawElementBonus("바람 뎀증 (Wind)", StatComp->GetFinalStat(EPetStatType::WindDamageBonus), ImVec4(0.5f, 1.0f, 0.6f, 1.0f));
						DrawElementBonus("독 뎀증 (Poison)", StatComp->GetFinalStat(EPetStatType::PoisonDamageBonus), ImVec4(0.6f, 0.2f, 0.8f, 1.0f));
						DrawElementBonus("출혈 뎀증 (Bleed)", StatComp->GetFinalStat(EPetStatType::BleedDamageBonus), ImVec4(0.9f, 0.1f, 0.1f, 1.0f));

						ImGui::EndTable();
					}
					ImGui::TreePop();
				}
			}
			ImGui::Spacing();

			// ==========================================
			// 3. 인벤토리 및 시너지 (이모티콘 제거 및 제거 액션 버튼 추가)
			// ==========================================
			UPetItemComponent* ItemComp = Pet->GetItemComponent();
			if (ItemComp)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.2f, 1.0f));
				bool bItemOpen = ImGui::TreeNodeEx("[ 보유 아이템 ] (Inventory)", ImGuiTreeNodeFlags_DefaultOpen);
				ImGui::PopStyleColor();

				if (bItemOpen)
				{
					const TArray<FPetItemInstance>& Items = ItemComp->GetOwnedItems();

					if (Items.IsEmpty())
					{
						ImGui::TextDisabled("  장착 중인 아이템이 없습니다.");
					}
					else
					{
						// 지연 제거를 위한 변수
						FGuid ItemInstanceToRemove;

						// 🌟 4열(액션 버튼 추가)로 확장
						if (ImGui::BeginTable("PetInventoryTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
						{
							ImGui::TableSetupColumn("아이템 이름", ImGuiTableColumnFlags_WidthStretch, 0.35f);
							ImGui::TableSetupColumn("시너지 태그", ImGuiTableColumnFlags_WidthStretch, 0.35f);
							ImGui::TableSetupColumn("고유 ID", ImGuiTableColumnFlags_WidthStretch, 0.15f);
							ImGui::TableSetupColumn("액션", ImGuiTableColumnFlags_WidthFixed, 75.0f); // 버튼 열 추가
							ImGui::TableHeadersRow();

							for (int32 ItemIdx = 0; ItemIdx < Items.Num(); ++ItemIdx)
							{
								const FPetItemInstance& Inst = Items[ItemIdx];
								if (Inst.LoadedData)
								{
									// 고유 ID 푸시 (아이템 삭제 버튼 식별용)
									ImGui::PushID(Inst.InstanceId.A);
									ImGui::TableNextRow();

									// 이름
									ImGui::TableSetColumnIndex(0);
									ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);
									ImGui::Text("%s", TCHAR_TO_UTF8(*Inst.LoadedData->ItemName.ToString()));

									// 태그
									ImGui::TableSetColumnIndex(1);
									ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);
									FString TagsStr = Inst.LoadedData->SynergyTags.ToStringSimple();
									if (TagsStr.IsEmpty()) ImGui::TextDisabled("-");
									else ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.5f, 1.0f), "%s", TCHAR_TO_UTF8(*TagsStr));

									// 디버그용 ID
									ImGui::TableSetColumnIndex(2);
									ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);
									ImGui::TextDisabled("%s...", TCHAR_TO_UTF8(*Inst.InstanceId.ToString().Left(8)));
									if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", TCHAR_TO_UTF8(*Inst.InstanceId.ToString()));

									// 🌟 액션 (제거 버튼)
									ImGui::TableSetColumnIndex(3);
									ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
									ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
									ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));

									if (ImGui::Button("제거 (Remove)", ImVec2(-FLT_MIN, 28.0f)))
									{
										ItemInstanceToRemove = Inst.InstanceId; // 루프 도중 배열이 깨지지 않도록 ID만 저장
									}

									ImGui::PopStyleColor(3);
									ImGui::PopID();
								}
							}
							ImGui::EndTable();
						}

						// 🌟 루프 밖에서 안전하게 아이템 제거 실행 (Access Violation 크래시 방지)
						if (ItemInstanceToRemove.IsValid())
						{
							PetManagerComponent->RemoveItemFromPet(Pet, ItemInstanceToRemove);
							UE_LOG(LogTemp, Log, TEXT("[Debug] Pet(%s)에서 아이템(ID:%s) 강제 제거됨"), *Pet->GetName(), *ItemInstanceToRemove.ToString());
						}
					}
					ImGui::TreePop();
				}
				ImGui::Spacing();

				// 활성화된 시너지 (이모티콘 제거 및 표준 기호 사용)
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.4f, 1.0f, 1.0f));
				bool bSynergyOpen = ImGui::TreeNodeEx("[ 활성화된 시너지 ] (Active Synergies)", ImGuiTreeNodeFlags_DefaultOpen);
				ImGui::PopStyleColor();

				if (bSynergyOpen)
				{
					const TArray<TObjectPtr<UPetSynergyDataAsset>>& Synergies = ItemComp->GetActiveSynergies();
					if (Synergies.IsEmpty())
					{
						ImGui::TextDisabled("  발동 중인 시너지가 없습니다.");
					}
					else
					{
						for (const UPetSynergyDataAsset* Synergy : Synergies)
						{
							if (Synergy)
							{
								ImGui::TextColored(ImVec4(0.8f, 0.4f, 1.0f, 1.0f), "  - [ON] %s", TCHAR_TO_UTF8(*Synergy->SynergyName.ToString()));

								if (ImGui::IsItemHovered())
								{
									ImGui::BeginTooltip();
									ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "[ 시너지 요구 조건 ]");
									for (const auto& Pair : Synergy->RequiredTagCounts)
									{
										ImGui::BulletText("%s : %d개 필요", TCHAR_TO_UTF8(*Pair.Key.ToString()), Pair.Value);
									}
									ImGui::EndTooltip();
								}
							}
						}
					}
					ImGui::TreePop();
				}
			}
			ImGui::Spacing();

			// ==========================================
			// 4. 최종 공격 제원 (이모티콘 제거 및 표준 기호 사용)
			// ==========================================
			UPetCombatComponent* CombatComp = Pet->GetCombatComponent();
			if (CombatComp && ItemComp)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
				bool bCombatOpen = ImGui::TreeNodeEx("[ 최종 공격 제원 ] (Combat Output)", ImGuiTreeNodeFlags_DefaultOpen);
				ImGui::PopStyleColor();

				if (bCombatOpen)
				{
					if (ImGui::BeginTable("PetCombatTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
					{
						FProjectileModifierData AggregatedProj = ItemComp->GetAggregatedProjectileModifier();
						FGameplayTag DominantTag = ItemComp->GetDominantElementTag();

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::TextDisabled("주 속성 (Dominant Element)");
						ImGui::TableSetColumnIndex(1);
						if (DominantTag.IsValid()) ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "%s", TCHAR_TO_UTF8(*DominantTag.ToString()));
						else ImGui::TextDisabled("무속성 (None)");

						FString ProjName = AggregatedProj.OverrideProjectileClass.IsNull() ?
							(CombatComp->DefaultProjectileClass ? CombatComp->DefaultProjectileClass->GetName() : "None") :
							AggregatedProj.OverrideProjectileClass.GetAssetName();
						ImGuiUtils::DrawRowText("발사체 클래스 (Projectile)", "%s", TCHAR_TO_UTF8(*ProjName));

						const char* PatternStr = "단발 (Single)";
						ImVec4 PatternColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
						switch (AggregatedProj.Pattern)
						{
						case EProjectilePattern::Spread: PatternStr = "산탄 (Spread)"; PatternColor = ImVec4(1.0f, 0.6f, 0.2f, 1.0f); break;
						case EProjectilePattern::Laser:  PatternStr = "레이저 (Laser)"; PatternColor = ImVec4(0.2f, 1.0f, 1.0f, 1.0f); break;
						case EProjectilePattern::Spiral: PatternStr = "나선 (Spiral)"; PatternColor = ImVec4(0.8f, 0.3f, 1.0f, 1.0f); break;
						default: break;
						}

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::TextDisabled("발사 패턴 (Pattern)");
						ImGui::TableSetColumnIndex(1); ImGui::TextColored(PatternColor, "%s", PatternStr);

						ImGuiUtils::DrawRowText("동시 발사 수 (Count)", "%d 발", FMath::Max(1, AggregatedProj.ProjectileCountAdd));
						ImGuiUtils::DrawRowText("크기 배율 (Size)", "x %.2f", AggregatedProj.SizeMultiplier);
						ImGuiUtils::DrawRowText("속도 배율 (Speed)", "x %.2f", AggregatedProj.SpeedMultiplier);

						ImGui::EndTable();
					}
					ImGui::TreePop();
				}
			}

			ImGui::Unindent(15.0f);
			ImGui::Spacing();
		}

		ImGui::PopID();
		ImGui::Separator();
	}
}