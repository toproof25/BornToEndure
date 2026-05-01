// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "imgui.h"
#include "ImGuiModule.h"     
#include "ImGuiDelegates.h" 

#include "DebugImGuiComponent.generated.h"


class APlayerCharacter;
class ABaseWeapon;
class UPlayerAnimInstance;
class UStatComponent;
class UCharacterMovementComponent;
class USpringArmComponent;
class UCameraComponent;
class UPetManagerComponent;
class UEnemyDetectorComponent;
class UPlayerExperienceComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BORNTOENDURE_API UDebugImGuiComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDebugImGuiComponent();

protected:
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	FImGuiDelegateHandle ImGuiDelegateHandle;
	void LoadKoreanFontToImGui();

	// Camera
	USpringArmComponent* SpringArmComp;
	UCameraComponent* CameraComp;

	// Player Component
	APlayerCharacter* PlayerCharacter;
	UPlayerAnimInstance* PlayerAnimInstance;
	UStatComponent* PlayerStatComponent;
	UCharacterMovementComponent* PlayerMovementComponent;
	UPetManagerComponent* PetManagerComponent;
	UEnemyDetectorComponent* EnemyDetectorComponent;
	UPlayerExperienceComponent* PlayerExperienceComponent;

	// Draw Debug ImGui Window
	void DebugDrawPlayerInfo();
	

	void DrawCameraInfo();
	void DrawPlayerCharacterInfo();
	void DrawAnimationBasic();
	void DrawWeaponInfo();
	void DrawStatInfo();
	void DrawEnemyDetectionInfo();
	void DrawExperienceInfo();

	// Player Character Movement Debug Functions
	void DrawMovement_Basic();
	void DrawMovement_State();
	void DrawMovement_Speed();
	void DrawMovement_Input();
	void DrawMovement_Physics();

	// 월드 및 서브시스템 전용 디버그 렌더링 함수
	void DrawSystemAndMemoryInfo();

	// 펫 시스템 전용 디버그 렌더링 함수
	void DrawPetInfo();

};
