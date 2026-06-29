// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/DefaultGameModeBase.h"

ADefaultGameModeBase::ADefaultGameModeBase()
{
}

void ADefaultGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	InitializeGame();
}

void ADefaultGameModeBase::InitializeGame()
{
	UE_LOG(LogTemp, Log, TEXT("GameModeBase Initialized: Setting up game state, spawning initial actors, etc."));
}
