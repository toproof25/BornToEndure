#include "Subsystem/EffectSubsystem.h"

#include "Data/SoundDataAsset.h"
#include "Data/NiagaraDataAsset.h"

#include "Engine/AssetManager.h" 
#include "Kismet/GameplayStatics.h"
#include "UObject/PrimaryAssetId.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

DEFINE_LOG_CATEGORY(LogEffectSubsystem);

void UEffectSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{

	Super::Initialize(Collection);

	// 이곳에서 필요한 이펙트 관련 초기화 작업을 수행
	UE_LOG(LogEffectSubsystem, Display, TEXT("EffectSubsystem Initialized"));
}


void UEffectSubsystem::PreloadEffectAssets(FPrimaryAssetId PrimaryAssetId)
{
	if (!PrimaryAssetId.IsValid())
	{
		UE_LOG(LogEffectSubsystem, Warning, TEXT("PreloadEffectAssets called with invalid PrimaryAssetId"));
		return;
	}

	FLoadedAsset& Asset = PreloadAsset.FindOrAdd(PrimaryAssetId);

	// TMap에 존재하면, 카운트를 증가시키고 바로 리턴
	if (Asset.Count > 0)
	{
		Asset.Count++;
		return;
	}

	// PreloadAsset에서 직접 값 가져오기 (최초에는 유효하지 않은 값)	
	TSharedPtr<FStreamableHandle>& StreamableHandle = Asset.StreamableHandle;

	// 유효하지 않으면(최초의 경우) 새로 로드 후 TMap에 저장
	if (!StreamableHandle.IsValid())
	{
		UAssetManager& AssetManager = UAssetManager::Get();
		StreamableHandle = AssetManager.LoadPrimaryAsset(PrimaryAssetId);
	}

	if (StreamableHandle.IsValid())
	{
		Asset.Count = 1;
	}
	else
	{
		PreloadAsset.Remove(PrimaryAssetId);
		UE_LOG(LogEffectSubsystem, Error, TEXT("PreloadEffectAssets Failed to load asset for %s"), *PrimaryAssetId.ToString());
	}
}

void UEffectSubsystem::UnloadEffectAssets(FPrimaryAssetId PrimaryAssetId)
{
	FLoadedAsset* Asset = PreloadAsset.Find(PrimaryAssetId);
	if (Asset)
	{
		Asset->Count--;

		if (Asset->Count <= 0)
		{
			if (Asset->StreamableHandle.IsValid())
			{
				Asset->StreamableHandle->ReleaseHandle(); // 핸들 해제
			}
			PreloadAsset.Remove(PrimaryAssetId);		   // TMap에서 제거
		}
	}
}


void UEffectSubsystem::SpawnSoundAtLocation(FName SoundName, FVector SoundSpawnLocation)
{
	// 설정한 ID "SoundDataAsset" + 에셋 파일명
	FPrimaryAssetType AssetType(FName(TEXT("SoundDataAsset")));
	FPrimaryAssetId AssetId(AssetType, SoundName);

	// 에셋 매니저 참조
	UAssetManager& AssetManager = UAssetManager::Get();
	USoundDataAsset* LoadedData = Cast<USoundDataAsset>(AssetManager.GetPrimaryAssetObject(AssetId));

	// 이미 메모리에 로드되어 있는지 체크 후 재생
	if (LoadedData && LoadedData->Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, LoadedData->Sound, SoundSpawnLocation);
		return;
	}

	// AssetId 비동기 로드 후 콜백
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
		UE_LOG(LogEffectSubsystem, Error, TEXT("Sound: Failed to load asset for %s"), *AssetId.ToString());
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

void UEffectSubsystem::SpawnNiagaraAtLocation(FName NiagaraName, FVector NiagaraSpawnLocation)
{
	// 설정한 ID "NiagaraDataAsset" + 에셋 파일명
	FPrimaryAssetType AssetType(FName(TEXT("NiagaraDataAsset")));
	FPrimaryAssetId AssetId(AssetType, NiagaraName);

	// 에셋 매니저 참조
	UAssetManager& AssetManager = UAssetManager::Get();
	UNiagaraDataAsset* LoadedData = Cast<UNiagaraDataAsset>(AssetManager.GetPrimaryAssetObject(AssetId));

	// 이미 메모리에 로드되어 있는지 체크 후 재생
	if (LoadedData && LoadedData->Niagara)
	{
		UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			LoadedData->Niagara,
			NiagaraSpawnLocation,
			FRotator::ZeroRotator,
			FVector(1.0f),
			true,  // bAutoDestroy
			true,  // bAutoActivate
			ENCPoolMethod::AutoRelease,
			true   // bPreCullCheck
		);
		return;
	}

	// AssetId 비동기 로드 후 콜백
	TSharedPtr<FStreamableHandle> Handle = AssetManager.LoadPrimaryAsset(AssetId);
	if (Handle.IsValid())
	{
		Handle->BindCompleteDelegate(
			FStreamableDelegate::CreateUObject(
				this,
				&UEffectSubsystem::OnNiagaraLoaded,
				AssetId,
				NiagaraSpawnLocation
			)
		);
	}
	else
	{
		UE_LOG(LogEffectSubsystem, Error, TEXT("Sound: Failed to load asset for %s"), *AssetId.ToString());
	}


}

void UEffectSubsystem::OnNiagaraLoaded(FPrimaryAssetId LoadedAssetId, FVector Location)
{
	UAssetManager& AssetManager = UAssetManager::Get();
	UNiagaraDataAsset* LoadedData = Cast<UNiagaraDataAsset>(AssetManager.GetPrimaryAssetObject(LoadedAssetId));
	if (LoadedData && LoadedData->Niagara)
	{
		UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			LoadedData->Niagara,
			Location,
			FRotator::ZeroRotator,
			FVector(1.0f),
			true,  // bAutoDestroy
			true,  // bAutoActivate
			ENCPoolMethod::AutoRelease,
			true   // bPreCullCheck
		);
	}
	else
	{
		UE_LOG(LogEffectSubsystem, Error, TEXT("Loaded asset invalid or missing Niagara for %s"), *LoadedAssetId.ToString());
	}
}
