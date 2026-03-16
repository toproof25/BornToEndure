// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultPlayerController.h"
#include "EnhancedInputSubsystems.h"

void ADefaultPlayerController::BeginPlay()
{
	Super::BeginPlay();

	check(GEngine != nullptr);

	// 로컬 플레이어 가져오기, (멀티플레이 시, 다른 유저 컨트롤러에서는 이 값이 null이 된다)
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer) return;

	// 로컬 플레이어에 입력 서브시스템을 가져옵니다.
	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsystem) return;

	// 입력 매핑 컨텍스트를 로컬 플레이어 서브시스템에 등록 (우선순위 0)
	Subsystem->AddMappingContext(DefaultMappingContext, 0);
	UE_LOG(LogTemp, Warning, TEXT("PlayerCharacter BeginPlay called. DefaultMappingContext"));
}
