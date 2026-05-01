// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/PetItemDataAsset.h"

#if !UE_BUILD_SHIPPING   
#include "ImGuiDelegates.h" 
#endif

#include "ItemPickupTestActor.generated.h"

class UPetItemDataAsset;
class UPetManagerComponent;
class APlayerCharacter;
class UStaticMeshComponent;
class UBoxComponent;
class UPrimitiveComponent;

UCLASS()
class BORNTOENDURE_API AItemPickupTestActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItemPickupTestActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	void RenderImGui();

	FTimerHandle FireTimerHandle;        // 타이머 핸들
	FImGuiDelegateHandle ImGuiDelegateHandle;

    // 에디터에서 DataAsset을 직접 참조 (테스트 단계에서는 하드 레퍼런스 허용)
    UPROPERTY(EditAnywhere, Category = "Item")
    TObjectPtr<UPetItemDataAsset> ItemData;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> CollisionComp;

	UPROPERTY()
	TObjectPtr<APlayerCharacter> PlayerRef;

	void GetItem();
};
