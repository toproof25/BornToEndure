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
	// 알파값 및 초기 윈도우 사이즈 지정
	ImGui::SetNextWindowBgAlpha(0.85f);
	ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiCond_FirstUseEver);

	// 윈도우 이름 변경
	ImGui::Begin("종합 디버그 패널 (System Debug Panel)");

	ImGui::Text("게임 내 다양한 시스템과 플레이어 상태를 실시간으로 모니터링합니다.");
	ImGui::Separator();

	// 🌟 탭 바(Tab Bar)를 도입하여 플레이어와 월드 시스템을 분리!
	if (ImGui::BeginTabBar("MainDebugTabBar"))
	{
		// ==============================================
		// 탭 1: 플레이어 전용 (Player)
		// ==============================================
		if (ImGui::BeginTabItem("플레이어 (Player)"))
		{
			DrawCameraInfo();
			DrawPlayerCharacterInfo();
			DrawAnimationBasic();
			DrawWeaponInfo();
			DrawStatInfo();

			if (PlayerMovementComponent && ImGui::CollapsingHeader("이동 (Movement)", ImGuiTreeNodeFlags_DefaultOpen))
			{
				DrawMovement_Basic();
				DrawMovement_State();
				DrawMovement_Speed();
				DrawMovement_Input();
				DrawMovement_Physics();
			}
			ImGui::EndTabItem();
		}

		// ==============================================
		// 탭 2: 월드 & 서브시스템 전용 (World & Memory)
		// ==============================================
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