
#include "Item/ItemPickupTestActor.h"
#include "Components/StaticMeshComponent.h"
#include "Component/PetManagerComponent.h"
#include "Character/Player/PlayerCharacter.h"
#include "Components/BoxComponent.h"

#if !UE_BUILD_SHIPPING
#include "imgui.h"
#include "ImGuiModule.h"     
#include "ImGuiDelegates.h" 
#endif

#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"


AItemPickupTestActor::AItemPickupTestActor()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComp"));
	RootComponent = CollisionComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	CollisionComp->SetGenerateOverlapEvents(true);
}

void AItemPickupTestActor::BeginPlay()
{
	Super::BeginPlay();

	if (FImGuiModule::IsAvailable())
	{
		FImGuiDelegate Delegate = FImGuiDelegate::CreateUObject(this, &AItemPickupTestActor::RenderImGui);
		ImGuiDelegateHandle = FImGuiModule::Get().AddWorldImGuiDelegate(Delegate);
	}

	PlayerRef = Cast<APlayerCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
}

void AItemPickupTestActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (FImGuiModule::IsAvailable() && ImGuiDelegateHandle.IsValid())
	{
		FImGuiModule::Get().RemoveImGuiDelegate(ImGuiDelegateHandle);
		ImGuiDelegateHandle.Reset();
	}
	Super::EndPlay(EndPlayReason);
}

void AItemPickupTestActor::RenderImGui()
{
	if (ImGui::Begin("Item Pickup Test"))
	{
		ImGui::Text("Item");
		ImGui::Separator();
		if (ItemData)
		{
			ImGui::Text("Item: %s", TCHAR_TO_ANSI(*GetName()));
			if (ImGui::Button("Get Item"))
			{
				GetItem();
			}
		}
		else
		{
			ImGui::Text("No Item Data Asset assigned. Please assign one in the Details panel.");
		}
	}
	ImGui::End();

}

void AItemPickupTestActor::GetItem()
{
	if (PlayerRef)
	{
		UE_LOG(LogTemp, Log, TEXT("Player picked up item"));
		if (UPetManagerComponent* PetMgr =
			PlayerRef->FindComponentByClass<UPetManagerComponent>())
		{
			if (ItemData)
			{
				// 이미 로드된 DataAsset이므로 직접 전달
				PetMgr->GiveItemToPet(PetMgr->GetActivePet(), ItemData);
				Destroy();
			}
		}
	}
}


