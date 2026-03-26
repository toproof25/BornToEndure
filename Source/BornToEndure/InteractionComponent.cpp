// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionComponent.h"
#include "Interactable.h"
#include "PlayerCharacter.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SphereShape = FCollisionShape::MakeSphere(15.f);
	QueryParams.AddIgnoredActor(Owner); // 플레이어 캐릭터는 무시 설정 
}


void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	Owner = Cast<APawn>(GetOwner());
	PlayerController = Cast<APlayerController>(Owner->GetController());
}

void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 1. 플레이어를 못찾을 경우
	if (Owner == nullptr) return;

	// 2. 플레이어 컨트롤러가 없으면 다시 시도 (BeginPlay에서 못찾을 수 있다고 함)
	if (PlayerController == nullptr)
	{
		PlayerController = Cast<APlayerController>(Owner->GetController());
		if (PlayerController == nullptr) return;
	}

	// 3. 플레이어가 상호작용 가능한 액터를 바라보고 있는지 확인
	FHitResult HitResult;
	bool bHit = PerformInteractionTrace(HitResult);

	if (bHit)
	{
		if (IInteractable* Interactable = Cast<IInteractable>(HitResult.GetActor()))
		{
			DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 15.f, 16, FColor::Green, false, -1.f);
		}
	}

}




bool UInteractionComponent::PerformInteractionTrace(FHitResult& OutHitResult)
{
	// 컨트롤러의 카메라 위치와 회전을 가져온다
	FVector CameraLocation;
	FRotator CameraRotation;
	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

	FVector TraceStart = CameraLocation;
	FVector TraceEnd = TraceStart + (CameraRotation.Vector() * 1000.f);

	return GetWorld()->SweepSingleByChannel(
		OutHitResult,
		TraceStart,
		TraceEnd,
		FQuat::Identity,
		ECC_GameTraceChannel1,
		SphereShape,
		QueryParams
	);
}

void UInteractionComponent::TestInteraction()
{
	FHitResult HitResult;
	bool bHit = PerformInteractionTrace(HitResult);

	if (bHit)
	{
		if (IInteractable* Interactable = Cast<IInteractable>(HitResult.GetActor()))
		{
			Interactable->Execute_Interact(HitResult.GetActor(), Cast<APlayerCharacter>(Owner));
			//DrawDebugLine(GetWorld(), HitResult.TraceStart, HitResult.TraceEnd, FColor::Red, false, 2.f);
		}
	}
}


