#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

// 데이터 애셋 헤더 포함
#include "Data/PetStatItemDataAsset.h"
#include "Data/PetProjectileItemDataAsset.h"

#if !UE_BUILD_SHIPPING   
#include "ImGuiDelegates.h" 
#endif

#include "PetItemDebugActor.generated.h"

class UPetManagerComponent;
class APetCompanionCharacter;

UCLASS()
class BORNTOENDURE_API APetItemDebugActor : public AActor
{
	GENERATED_BODY()

public:
	APetItemDebugActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// ImGui 메인 렌더링 함수
	void RenderImGui();

private:
#if !UE_BUILD_SHIPPING
	FImGuiDelegateHandle ImGuiDelegateHandle;
#endif

	// --- 윈도우 상태 관리 ---
	bool bIsWindowOpen = true;
	bool bIsLoadingAssets = true;

	// --- 로드된 아이템 보관 배열 ---
	UPROPERTY()
	TArray<UPetStatItemDataAsset*> LoadedStatItems;

	UPROPERTY()
	TArray<UPetProjectileItemDataAsset*> LoadedProjectileItems;

	// --- 펫 선택 관리 ---
	TWeakObjectPtr<APetCompanionCharacter> SelectedPet;

	// --- 내부 헬퍼 함수 ---
	void LoadAllPetItemsAsync();
	void OnItemsLoaded(TArray<FPrimaryAssetId> LoadedIds);

	// ImGui 그리기 헬퍼
	void DrawPetSelectionCombo(UPetManagerComponent* PetManager);
	void DrawStatItemsTab();
	void DrawProjectileItemsTab();

	// Enum 문자열 파싱 헬퍼
	template<typename TEnum>
	FString GetEnumDisplayName(const FString& EnumPath, TEnum EnumValue);

	// [추가] 로드된 텍스처가 가비지 컬렉터(GC)에 의해 삭제되는 것을 방지합니다.
	UPROPERTY()
	TArray<UTexture2D*> CachedIcons;
};

template<typename TEnum>
inline FString APetItemDebugActor::GetEnumDisplayName(const FString& EnumPath, TEnum EnumValue)
{
	if (const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, *EnumPath, true))
	{
		// 1. 메타데이터(DisplayName)가 있는지 먼저 확인합니다.
		FString DisplayName = EnumPtr->GetMetaData(TEXT("DisplayName"), static_cast<int32>(EnumValue));
		if (!DisplayName.IsEmpty())
		{
			return DisplayName; // "공격력", "고정값 추가" 반환
		}

		// 2. DisplayName이 없다면 일반 Enum 이름을 파싱해서 반환합니다.
		FString EnumString = EnumPtr->GetNameStringByValue(static_cast<int64>(EnumValue));
		FString Left, Right;
		if (EnumString.Split(TEXT("::"), &Left, &Right))
		{
			return Right;
		}
		return EnumString;
	}
	return TEXT("Unknown");
}
