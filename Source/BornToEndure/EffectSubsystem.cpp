#include "EffectSubsystem.h"

#include "SoundDataAsset.h"
#include "Engine/AssetManager.h" // 에셋 매니저 포함
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"

DEFINE_LOG_CATEGORY(LogEffectSubsystem);

void UEffectSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{

	Super::Initialize(Collection);
	// 이곳에서 필요한 이펙트 관련 초기화 작업을 수행
	UE_LOG(LogEffectSubsystem, Display, TEXT("EffectSubsystem Initialized"));
}

void UEffectSubsystem::SpawnSoundAtLocation(FString SoundName, FVector SoundSpawnLocation)
{
    // 설정한 ID "SoundDataAsset" + 에셋 파일명
    FPrimaryAssetType AssetType(FName(TEXT("SoundDataAsset")));
    FPrimaryAssetId AssetId(AssetType, FName(*SoundName));
    
    // 에셋 매니저 참조
    UAssetManager& AssetManager = UAssetManager::Get();
    USoundDataAsset* LoadedData = Cast<USoundDataAsset>(AssetManager.GetPrimaryAssetObject(AssetId));

    // 이미 메모리에 로드되어 있는지 체크 후 재생
    if (LoadedData && LoadedData->Sound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, LoadedData->Sound, SoundSpawnLocation);
        return;
    }


    // AssetId 에셋이 메모리에 있는지 확인하rh, 비동기 로드 후 콜백
    TSharedPtr<FStreamableHandle> Handle = AssetManager.LoadPrimaryAsset(AssetId);
    if (Handle.IsValid())
    {
        Handle->BindCompleteDelegate(
            FStreamableDelegate::CreateUObject(
                this,
                &UEffectSubsystem::OnSoundLoaded,
                AssetId,
                SoundSpawnLocation
            )
        );
    }
    else
    {
		UE_LOG(LogEffectSubsystem, Error, TEXT("Failed to load asset for %s"), *AssetId.ToString());
    }


}

void UEffectSubsystem::OnSoundLoaded(FPrimaryAssetId LoadedAssetId, FVector Location)
{
    UAssetManager& AssetManager = UAssetManager::Get();
    USoundDataAsset* LoadedData = Cast<USoundDataAsset>(AssetManager.GetPrimaryAssetObject(LoadedAssetId));
    if (LoadedData && LoadedData->Sound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, LoadedData->Sound, Location);
    }
    else
    {
        UE_LOG(LogEffectSubsystem, Error, TEXT("Loaded asset invalid or missing sound for %s"), *LoadedAssetId.ToString());
    }
}

