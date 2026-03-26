#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameFramework/Actor.h"
#include "ProjectilePoolSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogProjectilePoolSubsystem, Log, All);

class ABaseProjectile;

UCLASS()
class BORNTOENDURE_API UProjectilePoolSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // 월드 서브시스템 초기화 시 호출 (BeginPlay와 비슷한 느낌)
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    /**
     * @brief ABaseProjectile Class 타입으로 풀 초기화
     * @param ProjectileClass 발사체 클래스 타입
     * @param PoolSize 관리할 발사체 수
     */
    void InitializePoolForClass(TSubclassOf<ABaseProjectile> ProjectileClass, int32 PoolSize);

    /**
     * @brief 사용할 ABaseProjectile Class를 Pool에서 요청
     * @param ProjectileClass 요청할 ABaseProjectile Class 타입
     * @return 요청한 발사체 인스턴스에 대한 포인터를 반환합니다.
     */
    ABaseProjectile* RequestProjectile(TSubclassOf<ABaseProjectile> ProjectileClass);

    /**
     * @brief 사용한 ABulletProjectile 액터를 풀로 반환
     * @param Projectile 
     */
    void ReturnProjectile(ABaseProjectile* Projectile);


public:
    /**
     * @brief 특정 발사체 클래스의 풀 상태를 반환합니다. (디버그 및 UI 용도)
     * @param ProjectileClass 조회할 발사체 클래스
     * @param OutTotal 풀에 생성된 전체 액터 수
     * @param OutActive 현재 활성화되어 날아가고 있는 액터 수
     * @param OutInactive 풀에서 대기 중인(Hidden) 액터 수
     */
    void GetPoolStats(TSubclassOf<ABaseProjectile> ProjectileClass, int32& OutTotal, int32& OutActive, int32& OutInactive) const;

private:
    /**
     * @brief ABulletProjectile 타입과 실제 인스턴스 액터를 관리하는 배열
	 * @note Key: ABulletProjectile 클래스 타입, Value: ABulletProjectile 타입의 인스턴스 배열
     */
    TMap<TSubclassOf<ABaseProjectile>, TArray<ABaseProjectile*>> ProjectilePools;

    /**
     * @brief ABulletProjectile 타입과 풀 크기 관리 배열
	 * @note Key: ABulletProjectile 클래스 타입, Value: 해당 클래스 타입의 풀 크기
     */
    TMap<TSubclassOf<ABaseProjectile>, int32> PoolSizes;

	// World가 절대 nullptr이 아님을 보장 (이렇게 해야 안전하다고 함..?)
    UWorld* GetWorldChecked();
};