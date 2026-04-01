// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ObjectPoolSubsystem.generated.h"

class AActor;

DECLARE_LOG_CATEGORY_EXTERN(LogObjectPoolSubsystem, Log, All);

UCLASS()
class BORNTOENDURE_API UObjectPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// 월드 서브시스템 초기화 시 호출 (BeginPlay와 비슷한 느낌)
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * @brief AActor Class 타입으로 풀 초기화
	 * @param ActorClass 타입
	 * @param PoolSize 관리할 발사체 수
	 */
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	void InitializePoolForClass(TSubclassOf<AActor> ActorClass, int32 PoolSize);

	/**
	 * @brief 사용할 AActor Class를 Pool에서 요청
	 * @param ActorClass 요청할 AActor Class 타입
	 * @return 요청한 발사체 인스턴스에 대한 포인터를 반환합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	AActor* RequestPoolActor(TSubclassOf<AActor> ActorClass);

	/**
	 * @brief 사용한 AActor 액터를 풀로 반환
	 * @param PoolActor
	 */
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	void ReturnPoolActor(AActor* PoolActor);


public:
	/**
	 * @brief 특정 발사체 클래스의 풀 상태를 반환합니다. (디버그 및 UI 용도)
	 * @param ActorClass 조회할 클래스
	 * @param OutTotal 풀에 생성된 전체 액터 수
	 * @param OutActive 현재 활성화되어 날아가고 있는 액터 수
	 * @param OutInactive 풀에서 대기 중인(Hidden) 액터 수
	 */
	void GetPoolStats(TSubclassOf<AActor> ActorClass, int32& OutTotal, int32& OutActive, int32& OutInactive) const;

private:
	/**
	 * @brief AActor 타입과 실제 인스턴스 액터를 관리하는 배열
	 * @note Key: AActor 클래스 타입, Value: AActor 타입의 인스턴스 배열
	 */
	TMap<TSubclassOf<AActor>, TArray<AActor*>> ActorPools;

	/**
	 * @brief AActor 타입과 풀 크기 관리 배열
	 * @note Key: AActor 클래스 타입, Value: 해당 클래스 타입의 풀 크기
	 */
	TMap<TSubclassOf<AActor>, int32> PoolSizes;

	// World가 절대 nullptr이 아님을 보장 (이렇게 해야 안전하다고 함..?)
	UWorld* GetWorldChecked();
};