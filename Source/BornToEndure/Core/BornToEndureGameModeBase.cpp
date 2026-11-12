#include "Core/BornToEndureGameModeBase.h"
#include "BornToEndureGameModeBase.h"
#include "Actors/EnemySpawnDirector.h"
#include "Kismet/GameplayStatics.h"
#include "Character/Player/PlayerCharacter.h"
#include "Component/PlayerHealthComponent.h"

void ABornToEndureGameModeBase::StartGame()
{
	UE_LOG(LogTemp, Warning, TEXT("[ABornToEndureGameModeBase] 게임 시작!"));

	CurrentWave = 1;
	GameState = EGameState::Playing;
	StartPlayTime = GetWorld()->GetTimeSeconds();
	UpdateWaveDifficulty(CurrentWave);
	SetSpawnEnemies();

	GetWorldTimerManager().SetTimer(WaveTimerHandle, this, &ABornToEndureGameModeBase::StartWave, 10.0f, true);
	UE_LOG(LogTemp, Warning, TEXT("[ABornToEndureGameModeBase] 10초 뒤 %d 웨이브가 시작된다!"), CurrentWave);
}
void ABornToEndureGameModeBase::EndGame()
{
	GameState = EGameState::GameOver;
	if (EnemySpawnDirector)
	{
		EnemySpawnDirector->StopWeaveSpawning();
		//EnemySpawnDirector->KillAllEnemies();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ABornToEndureGameModeBase] EnemySpawnDirector가 설정되지 않았습니다."));
	}

	GetWorldTimerManager().ClearTimer(WaveTimerHandle);
	UE_LOG(LogTemp, Warning, TEXT("[ABornToEndureGameModeBase] 게임 종료!"));
}

void ABornToEndureGameModeBase::StartWave()
{
	UE_LOG(LogTemp, Warning, TEXT("[ABornToEndureGameModeBase] 웨이브 생성!"));

	if (EnemySpawnDirector)
	{
		EnemySpawnDirector->StartWeaveSpawning();
		GetWorldTimerManager().ClearTimer(WaveTimerHandle);
		GetWorldTimerManager().SetTimer(WaveTimerHandle, this, &ABornToEndureGameModeBase::EndWave, 60.0f, true);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ABornToEndureGameModeBase] EnemySpawnDirector가 설정되지 않았습니다."));
	}
}
void ABornToEndureGameModeBase::EndWave()
{
	EnemySpawnDirector->StopWeaveSpawning();

	UE_LOG(LogTemp, Warning, TEXT("[ABornToEndureGameModeBase] 웨이브 종료!"));
	GetWorldTimerManager().ClearTimer(WaveTimerHandle);

	// 웨이브 증가 및 보상 등의 로직 추가 가능
	CurrentWave++;
	UpdateWaveDifficulty(CurrentWave);
	SetSpawnEnemies();
	GetWorldTimerManager().SetTimer(WaveTimerHandle, this, &ABornToEndureGameModeBase::StartWave, 10.0f, true);
	UE_LOG(LogTemp, Warning, TEXT("[ABornToEndureGameModeBase] 10초 뒤 %d 웨이브가 다시 시작된다!"), CurrentWave);
}

bool ABornToEndureGameModeBase::SkipToNextWave()
{
	EndWave();
	StartWave();
	return true;
}

void ABornToEndureGameModeBase::SetSpawnEnemies()
{
	EnemySpawnDirector->SetSpawnEnemies(CurrentWave, CurrentDifficulty);
}


void ABornToEndureGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	EnemySpawnDirector = Cast<AEnemySpawnDirector>(UGameplayStatics::GetActorOfClass(GetWorld(), AEnemySpawnDirector::StaticClass()));
	if (EnemySpawnDirector)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ABornToEndureGameModeBase] EnemySpawnDirector가 설정되었습니다."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ABornToEndureGameModeBase] EnemySpawnDirector가 설정되지 않았습니다."));
	}

	// 플레이어 사망 시 Delegate 구독
	APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	UPlayerHealthComponent* PlayerHealthComp = Player ? Player->FindComponentByClass<UPlayerHealthComponent>() : nullptr;
	if (PlayerHealthComp)
	{
		PlayerHealthComp->OnPlayerDeath.AddUObject(this, &ABornToEndureGameModeBase::EndGame);
	}

	CurrentWave = 1;
	StartPlayTime = 0.0f;
	//CurrentPlayTime = 0.0f;
	GameState = EGameState::WatingToStart;


}

void ABornToEndureGameModeBase::UpdateWaveDifficulty(int32 Wave)
{
	HealthCurve.Eval(
		static_cast<float>(Wave),
		&CurrentDifficulty.HealthMultiplier,
		TEXT("HealthCurve")
	);

	GoldCurve.Eval(
		static_cast<float>(Wave),
		&CurrentDifficulty.GoldMultiplier,
		TEXT("GoldCurve")
	);

	ExpCurve.Eval(
		static_cast<float>(Wave),
		&CurrentDifficulty.ExpMultiplier,
		TEXT("ExpCurve")
	);

	UE_LOG(LogTemp, Warning, TEXT("[ABornToEndureGameModeBase] 웨이브 %d 난이도 업데이트: 체력 배율 %.2f, 골드 배율 %.2f, 경험치 배율 %.2f"),
		Wave,
		CurrentDifficulty.HealthMultiplier,
		CurrentDifficulty.GoldMultiplier,
		CurrentDifficulty.ExpMultiplier
	);
}
