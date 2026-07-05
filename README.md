# BornToEndure

> Unreal Engine 5.4 기반 3D 생존 액션 / 자동 전투 / Pet 성장 시스템 프로토타입

**BornToEndure**는 한국 배경의 생존 액션, 뱀서라이크, 디펜스, 로그라이트 성장 요소를 결합하는 것을 목표로 개발 중인 Unreal Engine 5 C++ 프로젝트입니다.

플레이어가 직접 모든 공격을 수행하는 구조가 아니라, 플레이어를 따라다니는 여러 **Pet**이 적을 탐지하고 자동으로 공격하며, 레벨업마다 획득하는 아이템을 Pet에게 적용해 전투 방식과 스탯을 변화시키는 구조를 중심으로 설계하고 있습니다.

이 프로젝트는 단순 기능 구현보다 **UE5 C++ 게임플레이 구조 설계**, **컴포넌트 기반 책임 분리**, **DataAsset / DataTable 기반 데이터 주도 설계**, **Behavior Tree AI**, **Object Pooling 최적화**, **Delegate 기반 이벤트 흐름**, **UMG UI**, **ImGui 디버깅**, **Unreal Insights 기반 성능 분석**을 직접 구현하고 검증하는 데 초점을 둡니다.

---

## 목차

1. [프로젝트 개요](#프로젝트-개요)
2. [기술 스택](#기술-스택)
3. [핵심 게임 루프](#핵심-게임-루프)
4. [주요 시스템](#주요-시스템)
5. [아키텍처](#아키텍처)
6. [개발일지 기반 구현 흐름](#개발일지-기반-구현-흐름)
7. [현재 구현 상태](#현재-구현-상태)
8. [실행 방법](#실행-방법)
9. [개발 목표](#개발-목표)

---

## 프로젝트 개요

| 항목 | 내용 |
|---|---|
| 프로젝트명 | BornToEndure |
| 한글 가제 | 버팀의 민족 |
| 엔진 | Unreal Engine 5.4 |
| 개발 방식 | C++ 중심, Blueprint 보조 |
| 장르 목표 | 3D 생존 액션 / 자동 전투 / Pet 성장 / 디펜스 / 로그라이트 |
| 핵심 플레이 | Player 주변에서 Pet들이 적을 탐지하고 자동 공격, 레벨업 시 Pet 강화 아이템 선택 |
| 개발 목적 | UE5 C++ 구조 설계 학습, 포트폴리오, 향후 배포 가능한 프로토타입 제작 |

---

## 기술 스택

| 분류 | 사용 기술 |
|---|---|
| Engine | Unreal Engine 5.4 |
| Language | C++ |
| Input | Enhanced Input |
| Character | CharacterMovementComponent, SpringArm, CameraComponent |
| Animation | Animation Blueprint, AnimInstance, Blend Space, Control Rig, IK |
| AI | AIController, Behavior Tree, Blackboard, BTTask, BTService, NavMesh |
| Combat | ApplyDamage, TakeDamage, ProjectileMovementComponent |
| VFX / SFX | Niagara, SoundBase, PrimaryDataAsset, AssetManager |
| Data | DataAsset, PrimaryDataAsset, DataTable, GameplayTags |
| UI | UMG, UserWidget, ProgressBar, Button, Image, Widget Composition |
| System | ActorComponent, WorldSubsystem, GameInstanceSubsystem, PlayerState |
| Optimization | Object Pooling, Niagara Pooling, Timer 기반 처리, 캐시 공유 |
| Debug / Profiling | Unreal ImGui, Unreal Insights, UE_LOG |

---

## 핵심 게임 루프

```text
적 스폰
  ↓
Player 주변으로 적 접근
  ↓
Player의 EnemyDetectorComponent가 주변 적 목록 캐싱
  ↓
Pet들이 캐시된 적 목록을 공유하여 각자 타겟 선정
  ↓
PetCombatComponent가 스탯 + 아이템 정보를 조합하여 공격 정보 생성
  ↓
ObjectPoolSubsystem에서 발사체 요청
  ↓
Projectile이 적에게 발사 / 충돌 / 데미지 적용
  ↓
Enemy 사망 시 FEnemyRewardPayload Delegate 방송
  ↓
PlayerExperienceComponent가 경험치 / 골드 획득
  ↓
레벨업 시 LevelUpRewardWidget 표시
  ↓
Pet 선택 + Item 선택
  ↓
선택한 Item을 Pet에게 적용하여 스탯 또는 공격 방식 변경
```

---

## 주요 시스템

### 1. Player 기본 조작 시스템

초기 프로젝트는 Blank C++ 프로젝트에서 시작해, 직접 3인칭 캐릭터 조작을 구성했습니다.

구현 요소:

- `APlayerCharacter`
- `ADefaultPlayerController`
- `ADefaultGameModeBase`
- Enhanced Input 기반 `IA_Move`, `IA_Look`, `IA_Jump`, `IA_Sprint`, `IA_Interaction`
- `AddMovementInput` 기반 이동
- `Jump()` 기반 점프
- `SpringArmComponent` + `CameraComponent` 기반 3인칭 카메라
- PlayerController에서 Input Mapping Context 등록

초기에는 Character 내부에서 입력을 모두 처리했지만, 이후 확장성을 위해 **Input Mapping Context 등록은 PlayerController**, 실제 움직임은 Character에서 처리하는 구조로 분리했습니다.

---

### 2. 이동 속도 / Stat Component

달리기 기능은 `CharacterMovementComponent::MaxWalkSpeed`를 조절하여 구현했습니다.

단순히 PlayerCharacter 내부에 속도 값을 하드코딩하지 않고, 추후 아이템, 버프, 디버프, 상태 이상에 의해 값이 변할 수 있도록 `StatComponent`로 분리했습니다.

```text
PlayerCharacter
  └─ 입력 수신
      └─ StatComponent에 속도 재계산 요청
          └─ CharacterMovementComponent의 MaxWalkSpeed 갱신
```

---

### 3. Animation / IK / Control Rig

Player 애니메이션은 C++ `UPlayerAnimInstance`를 기반으로 현재 속도와 점프 상태를 계산하고, Animation Blueprint에서 이를 사용합니다.

구현 요소:

- `UPlayerAnimInstance`
- `GroundSpeed`
- `bIsFalling`
- Idle / Walk / Run Blend Space
- Jump Start / Loop / End State Machine
- 무기 장착 후 왼손 위치 보정을 위한 IK
- `Two Bone IK` 테스트
- Control Rig 기반 IK 테스트
- 무기 Mesh의 `LHIK` Socket을 기준으로 왼손 위치 보정

무기를 장착하지 않았을 때 IK가 불필요하게 적용되는 문제는 Alpha 값을 두어 무기 장착 여부에 따라 IK 적용 비율을 제어했습니다.

---

### 4. Interaction Interface / Trace 시스템

모든 상호작용 가능한 Actor를 공통적으로 처리하기 위해 Unreal Interface를 도입했습니다.

구현 요소:

- `UInteractable` / `IInteractable`
- `Interact_Implementation()`
- `Execute_Interact()`
- `UInteractionComponent`
- 카메라 중앙 기준 Sphere Trace
- Interaction 전용 Collision Channel
- F 키 입력 시 상호작용 실행

구조:

```text
Player가 F 입력
  ↓
InteractionComponent가 카메라 방향으로 Trace
  ↓
Hit Actor가 IInteractable 구현 여부 확인
  ↓
Execute_Interact 호출
```

이를 기반으로 무기 획득, 아이템 상호작용, 추후 오브젝트 상호작용까지 확장할 수 있도록 설계했습니다.

---

### 5. Weapon 장착 / Socket / IK

바닥에 있는 무기를 바라보고 F를 누르면 Player의 오른손 Socket에 무기가 장착됩니다.

구현 요소:

- `AWeaponBase`
- `EWeaponType`
- `WeaponSocket`
- `LHIK` Socket
- 무기 장착 시 Collision / Physics 비활성화
- `AttachToComponent()`로 Player Mesh Socket에 부착
- Player가 현재 장착 중인 무기를 저장
- AnimInstance가 현재 무기의 LHIK Transform을 읽어 왼손 IK에 반영

---

### 6. Projectile / Damage 시스템

발사체는 `ABaseProjectile`을 기반으로 구현했습니다.

구현 요소:

- `ABaseProjectile`
- `ABulletProjectile`
- `ProjectileMovementComponent`
- `SphereComponent` 충돌
- `ApplyDamage` / `TakeDamage`
- Hit 시 Sound / Niagara 재생
- 수명 종료 또는 충돌 시 Pool 반환

발사체는 다양한 공격 방식의 공통 기반이 될 수 있도록 추상화했으며, 이후 Pet 공격에서도 동일한 발사체 시스템을 재사용합니다.

---

### 7. Object Pooling 시스템

발사체처럼 자주 생성되고 제거되는 Actor는 Spawn / Destroy 반복 비용을 줄이기 위해 `ObjectPoolSubsystem`으로 관리합니다.

핵심 기능:

- `InitializePoolForClass()`
- `RequestPoolActor()`
- `ReturnPoolActor()`
- `RemovePoolActor()`
- `IPoolable` 인터페이스 기반 Activate / Deactivate

구조:

```text
전투 시작 전 발사체 Pool 초기화
  ↓
공격 시 Pool에서 비활성 Actor 요청
  ↓
위치 / 회전 / 공격 정보 설정
  ↓
발사체 활성화
  ↓
충돌 또는 수명 종료 시 비활성화 후 Pool 반환
```

---

### 8. EffectSubsystem / Sound / Niagara 관리

사운드와 Niagara 이펙트는 각 Actor가 직접 로드하고 재생하지 않고, `EffectSubsystem`에서 중앙 관리합니다.

구현 요소:

- `UEffectSubsystem`
- `SoundDataAsset`
- `NiagaraDataAsset`
- `FPrimaryAssetId`
- `AssetManager`
- `PreloadEffectAssets()`
- `UnloadEffectAssets()`
- Delegate 기반 이펙트 요청

초기에는 요청 시점에 이펙트를 로드했지만, 첫 재생 순간 딜레이가 발생할 수 있어 필요한 Actor / Component의 BeginPlay에서 Preload하고 EndPlay에서 Unload하는 구조로 개선했습니다.

---

### 9. Niagara Pooling / Unreal Insights 최적화

Niagara 이펙트는 Actor 기반 Pooling도 고려했지만, UE5의 `ENCPoolMethod::AutoRelease`를 우선 적용했습니다.

Unreal Insights를 활용하여 `ENCPoolMethod::None`과 `AutoRelease`의 성능 차이를 비교했고, 의미 있는 차이를 확인한 후 `EffectSubsystem`의 Niagara 재생에 반영했습니다.

---

### 10. Pet AI 시스템

Pet은 Player를 따라다니며 독립적으로 적을 공격하는 Actor입니다.

구현 요소:

- `APetCompanionCharacter`
- `PetCompanionAIController`
- `Behavior Tree`
- `Blackboard`
- `BTService_PetRadar`
- `BTTask_PetAttack`
- `BTTask_MoveToAttackPosition`
- NavMesh 기반 이동

기본 행동:

1. Player와의 거리를 확인한다.
2. 너무 멀면 Player에게 복귀한다.
3. 주변 적을 탐지한다.
4. 가장 가까운 적을 Target으로 설정한다.
5. 공격 범위 안이면 공격한다.
6. 공격 범위 밖이면 공격 가능한 위치로 이동한다.

---

### 11. EnemyDetectorComponent 기반 탐지 최적화

초기에는 각 Pet이 개별적으로 `OverlapMulti`를 호출하는 구조를 고려했습니다. 하지만 Pet이 많아질수록 탐지 비용이 증가하므로 Player에 `EnemyDetectorComponent`를 두어 주변 적을 한 번만 탐지하고, 모든 Pet이 그 결과를 공유하도록 변경했습니다.

```text
기존 구조:
Pet N마리 → N번 Overlap 검사

개선 구조:
Player 1회 Overlap 검사 → Pet N마리가 캐시된 적 목록 공유
```

---

### 12. Pet Combat / Stat / Item Component 구조

Pet 시스템은 기능별 Component로 분리했습니다.

| Component | 역할 |
|---|---|
| `PetCombatComponent` | 공격 타이머, 공격 정보 생성, 발사체 요청, 공격 실행 |
| `PetStatComponent` | 기본 스탯과 수정자 기반 최종 스탯 계산 |
| `PetItemComponent` | Pet이 보유한 아이템, 시너지, 발사체 수정자 관리 |
| `PetManagerComponent` | Player가 보유한 Pet 목록 관리, 아이템 지급 |

`PetCombatComponent`는 `PetStatComponent`, `PetItemComponent`를 직접 참조하지 않고 Interface를 통해 데이터를 가져옵니다.

```text
PetCombatComponent
  ├─ IPetStatProviderInterface
  └─ IPetItemProviderInterface
```

이를 통해 전투 컴포넌트가 스탯 / 아이템 컴포넌트 구현에 강하게 의존하지 않도록 했습니다.

---

### 13. DataAsset / GameplayTags 기반 Pet / Item / Synergy

Pet과 Item은 코드에 직접 고정하지 않고 DataAsset 기반으로 정의합니다.

구현 요소:

- `PetBaseDataAsset`
- `PetItemDataAsset`
- `PetStatItemDataAsset`
- `PetProjectileItemDataAsset`
- `PetSynergyDataAsset`
- `GameplayTags`
- `FStatModifier`
- `EStatModifierType`
- `EPetStatType`

아이템은 크게 두 가지 방향으로 작동합니다.

1. **Stat 변경**
   - 공격력 증가
   - 공격 속도 증가
   - 치명타 확률 증가
   - 속성 데미지 증가

2. **Projectile 변경**
   - 발사체 개수 증가
   - 발사체 크기 변경
   - 발사체 속도 변경
   - 단발 / 확산 / 나선형 패턴 변경

---

### 14. Enemy 사망 보상 / Kill 통계

적이 사망하면 `FEnemyRewardPayload`를 Delegate로 방송합니다.

포함 데이터:

- 경험치 보상
- 골드 보상
- 마지막으로 처치한 Pet ID
- Pet별 누적 데미지 맵
- 총 받은 데미지

처리 흐름:

```text
Enemy TakeDamage
  ↓
Pet별 데미지 기록
  ↓
Enemy 사망
  ↓
OnEnemyKilled Broadcast
  ↓
PlayerExperienceComponent가 보상 수신
  ↓
CombatPlayerState에 Pet별 데미지 / Kill 통계 누적
```

통계는 Player Pawn이 아닌 `ACombatPlayerState`에 저장하여 Player 생명주기와 분리했습니다.

---

### 15. Enemy / Item DataTable 관리

Enemy와 Item은 DataTable을 통해 관리합니다.

Enemy DataTable에는 다음 값을 정의합니다.

- Enemy ID
- 이름
- 설명
- Spawn Level
- Health
- AttackPower
- DefensePower
- MovementSpeed
- RewardExp
- RewardGold
- EnemyDataAsset

Item DataTable에는 다음 값을 정의합니다.

- Item ID
- 이름
- 설명
- 아이콘
- Item DataAsset

Item은 `ItemPoolSubsystem`에서 GameInstance 시작 시 캐싱하고, 레벨업 보상 시 랜덤 아이템 목록을 반환합니다.

---

### 16. Level Up / Reward UI

적 처치로 경험치를 얻고, 일정 수치 이상이 되면 레벨업합니다.

레벨업 시 Delegate를 통해 UI가 반응합니다.

구현 요소:

- `PlayerExperienceComponent`
- `OnChangeExpDelegate`
- `OnLevelUpDelegate`
- `PlayerHUDWidget`
- `LevelUpRewardWidget`
- `PetEntryWidget`
- `ItemEntryWidget`
- `ItemPoolSubsystem`
- `FLevelUpDataBundle`

흐름:

```text
경험치 획득
  ↓
OnChangeExpDelegate Broadcast
  ↓
HUD 경험치 Bar 갱신
  ↓
레벨업 조건 달성
  ↓
OnLevelUpDelegate Broadcast
  ↓
PlayerController가 Pet 목록 + 랜덤 Item 목록 수집
  ↓
LevelUpRewardWidget 생성 및 데이터 주입
  ↓
Pet / Item 선택
  ↓
선택한 Item을 선택한 Pet에게 적용
```

UI는 Player 상태를 직접 수정하지 않고, PlayerController가 중간에서 데이터를 모아 주입하는 Mediator 형태로 구성했습니다.

---

### 17. 직관적인 UI / Icon 비동기 로드

Item과 Pet 선택 UI에는 아이콘과 선택 강조 효과를 추가했습니다.

구현 요소:

- `TSoftObjectPtr<UTexture2D>` 기반 아이콘 참조
- AssetManager / StreamableManager 기반 비동기 로드
- `UImage::SetBrushFromTexture()`
- 선택된 Pet / Item Widget 캐싱
- 선택 상태에 따른 Border 색상 변경

UI Texture는 `UserInterface2D (RGBA)` 압축 설정을 사용하여 UI에서 선명하게 보이도록 구성했습니다.

---

### 18. Player 피격 / 사망 처리

적이 Player와 충돌하면 일정 공격 주기마다 Player에게 데미지를 줍니다.

구현 요소:

- `UPlayerHealthComponent`
- PlayerCharacter의 `TakeDamage()` 위임 구조
- Enemy의 AttackRange Collision
- `ApplyDamage()` 호출
- 공격 쿨타임 Timer
- `OnPlayerDeath` Delegate
- PlayerController의 사망 처리 구독

흐름:

```text
Enemy AttackRange에 Player 진입
  ↓
AttackPlayer 호출
  ↓
UGameplayStatics::ApplyDamage
  ↓
PlayerCharacter::TakeDamage
  ↓
PlayerHealthComponent::HealthTakeDamage
  ↓
체력 0 이하
  ↓
OnPlayerDeath Broadcast
  ↓
PlayerController가 사망 처리
```

---

## 아키텍처

```text
BornToEndure
│
├─ Core
│  ├─ DefaultPlayerController
│  ├─ DefaultGameModeBase
│  └─ BornToEndureGameInstance
│
├─ Character
│  ├─ PlayerCharacter
│  ├─ BaseEnemyCharacter
│  └─ PetCompanionCharacter
│
├─ Component
│  ├─ StatComponent
│  ├─ InteractionComponent
│  ├─ EnemyDetectorComponent
│  ├─ PlayerExperienceComponent
│  ├─ PlayerHealthComponent
│  ├─ PetManagerComponent
│  ├─ PetStatComponent
│  ├─ PetItemComponent
│  └─ PetCombatComponent
│
├─ AI
│  └─ Pet
│     ├─ BTService_PetRadar
│     ├─ BTTask_PetAttack
│     └─ BTTask_MoveToAttackPosition
│
├─ Item
│  ├─ Weapon
│  └─ Projectile
│
├─ Data
│  ├─ DataAsset
│  ├─ DataTableRow
│  └─ GameTypes
│
├─ Subsystem
│  ├─ ObjectPoolSubsystem
│  ├─ EffectSubsystem
│  └─ ItemPoolSubsystem
│
├─ UI
│  ├─ PlayerHUDWidget
│  ├─ LevelUpRewardWidget
│  ├─ PetEntryWidget
│  └─ ItemEntryWidget
│
└─ PlayerState
   └─ CombatPlayerState
```

---

## 개발일지 기반 구현 흐름

| 번호 | 주제 | 핵심 내용 |
|---:|---|---|
| 1 | C++ Project / Enhanced Input / 3인칭 이동 | Blank C++ 프로젝트 생성, Enhanced Input, CharacterMovement, SpringArm, Camera 구현 |
| 2 | PlayerController / GameModeBase | Input Mapping Context를 PlayerController로 분리하고 GameModeBase 설정 |
| 3 | Sprint / StatComponent | 달리기 속도 조절, Stat 관련 책임을 ActorComponent로 분리 |
| 4 | 걷기 / 달리기 / 점프 애니메이션 | AnimInstance, Blend Space, State Machine 기반 기본 애니메이션 적용 |
| 5 | Interface / Trace | IInteractable, InteractionComponent, Sphere Trace 기반 상호작용 구조 구현 |
| 6 | 무기 상호작용 / 장착 / IK | WeaponSocket, LHIK Socket, Control Rig / IK 기반 무기 장착 테스트 |
| 7 | 무기 애니메이션 방향성 | 무기 장착 후 사격 자세와 IK 보정 방향 정리 |
| 8 | 발사체 기본 구조 | BaseProjectile / BulletProjectile 기반 투사체 구조 구현 |
| 9 | 공격 / 피격 처리 | Projectile 충돌, ApplyDamage / TakeDamage 흐름 구성 |
| 10 | 범용 Actor Pooling | IPoolable, ObjectPoolSubsystem 기반 발사체 재사용 구조 구현 |
| 11 | ImGui 디버그 | Player / Pool / Effect 상태 확인용 디버그 윈도우 구축 |
| 12 | Enemy 기본 구조 | BaseEnemyCharacter, 피격 / 사망 / 보상 확장 기반 작성 |
| 13 | EffectSubsystem | Sound / Niagara 재생을 WorldSubsystem으로 중앙화 |
| 14 | Niagara Pooling | ENCPoolMethod 기반 Niagara Pooling 성능 비교 및 적용 |
| 15 | Projectile / Enemy Effect | 발사체와 Enemy 피격 시 Sound / Niagara 연동 |
| 16 | Pet Pawn / AIController | Player 주변을 따라다니는 Pet Character / AIController / NavMesh 테스트 |
| 17 | Pet Behavior Tree | Blackboard / Behavior Tree 기반 추적 / 탐지 구조 적용 |
| 18 | BT 공격 노드 | Blueprint BTTask로 Pet 발사체 공격 구현 및 성공 판정 처리 |
| 19 | BTTask / BTService C++화 | Pet 공격 Task와 탐지 Service를 C++로 전환, 성능 비교 |
| 20 | Projectile 발사 책임 분리 | 발사체 속도 설정 책임을 PetCombatComponent에서 Projectile 내부로 이동 |
| 21 | Effect Preload / Unload | AssetManager, PrimaryAssetId, StreamableHandle 기반 이펙트 로드 관리 |
| 22 | Pet System 구조 | DataAsset / GameplayTags / Component / Interface 기반 Pet 시스템 설계 |
| 23 | Enemy 사망 보상 | FEnemyRewardPayload와 Delegate 기반 경험치 / 골드 보상 전달 |
| 24 | Pet별 Kill 통계 | PlayerState에 Pet별 데미지 / Kill 통계 누적 |
| 25 | Enemy DataTable | 적 스탯과 보상을 DataTable로 정의하고 Spawner에서 적용 |
| 26 | Item DataTable | ItemDataRow와 ItemPoolSubsystem 기반 아이템 로드 / 지급 구조 준비 |
| 27 | Level Up System | 경험치 누적, 레벨 증가, 레벨업 조건 처리 구현 |
| 28 | Level Up Reward UI | Delegate 기반 HUD 갱신, LevelUpRewardWidget, Pet / Item 선택 구현 |
| 29 | UI Icon / 선택 강조 | TSoftObjectPtr 아이콘 비동기 로드, 선택된 Pet / Item 시각화 |
| 30 | Player 피격 / 사망 | Enemy 충돌 시 ApplyDamage, PlayerHealthComponent, 사망 Delegate 구현 |

---

## 현재 구현 상태

### 완료

- [x] UE5.4 C++ 프로젝트 구성
- [x] Enhanced Input 기반 이동 / 시점 / 점프 / 달리기
- [x] PlayerController / GameModeBase 분리
- [x] 기본 캐릭터 애니메이션
- [x] Interaction Interface / Trace
- [x] 무기 장착 / Socket / IK 테스트
- [x] 발사체 기본 구조
- [x] ApplyDamage / TakeDamage 기반 데미지 처리
- [x] ObjectPoolSubsystem
- [x] EffectSubsystem
- [x] Sound / Niagara DataAsset
- [x] Effect Preload / Unload
- [x] Niagara Pooling 테스트
- [x] Pet Character / AIController
- [x] Behavior Tree / Blackboard
- [x] C++ BTTask / BTService
- [x] EnemyDetectorComponent 기반 탐지 캐시 공유
- [x] PetCombatComponent / PetStatComponent / PetItemComponent 분리
- [x] GameplayTags 기반 시너지 설계
- [x] Enemy 사망 보상 Delegate
- [x] PlayerExperienceComponent
- [x] CombatPlayerState 기반 Pet별 데미지 통계
- [x] Enemy DataTable
- [x] Item DataTable
- [x] ItemPoolSubsystem
- [x] Player HUD
- [x] Level Up Reward Widget
- [x] Pet / Item 선택 UI
- [x] UI Icon 비동기 로드
- [x] PlayerHealthComponent
- [x] Enemy 충돌 기반 Player 피격 처리
- [x] ImGui 디버그 윈도우

### 진행 예정

- [ ] Enemy Wave / Stage 진행 구조
- [ ] GameMode 중심 게임 진행 Delegate 설계
- [ ] Boss Enemy 구현
- [ ] Pet별 고유 스킬 구현
- [ ] Item / Synergy 밸런싱
- [ ] UI / UX 정리
- [ ] Game Over UI
- [ ] 세이브 / 로드 구조
- [ ] 패키징 및 배포 빌드 검증

---

## 실행 방법

> 이 프로젝트는 Unreal Engine 5.4 기준입니다.

```bash
git clone https://github.com/toproof25/BornToEndure.git
```

1. `BornToEndure.uproject` 우클릭
2. `Generate Visual Studio project files` 실행
3. Visual Studio 2022에서 프로젝트 빌드
4. `BornToEndure.uproject` 실행
5. Unreal Editor에서 테스트 맵 실행

---

## 개발 목표

이 프로젝트는 단순히 기능을 많이 붙이는 것보다, UE5 C++ 프로젝트에서 실제로 중요한 구조적 문제를 직접 겪고 해결하는 것을 목표로 합니다.

중점적으로 학습하고 검증하는 내용:

- UE5 C++ Actor / Component 구조
- PlayerController / GameMode / PlayerState / GameInstance 역할 분리
- Enhanced Input 기반 입력 시스템
- CharacterMovement와 애니메이션 연동
- Interface와 Trace 기반 상호작용
- Object Pooling 기반 전투 최적화
- WorldSubsystem / GameInstanceSubsystem 활용
- AssetManager와 PrimaryDataAsset 기반 리소스 관리
- Behavior Tree와 C++ BTTask / BTService
- Delegate 기반 이벤트 흐름
- DataAsset / DataTable 기반 데이터 주도 설계
- UMG UI 데이터 주입 구조
- ImGui와 Unreal Insights 기반 디버깅 / 프로파일링

---

## 한 줄 요약

**BornToEndure는 UE5 C++ 기반으로 Pet 자동 전투, 데이터 주도형 성장, Object Pooling 최적화, Behavior Tree AI, Delegate 기반 UI 흐름을 직접 구현하며 게임 클라이언트 구조 설계를 검증하는 프로젝트입니다.**
