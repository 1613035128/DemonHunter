# 相机系统 — 设计方案

> 项目：DemonHunter | 引擎：Unreal Engine 5.7 | 日期：2026-05-23 | 版本：v1.0
>
> 本文档是设计规格，不包含代码实现。实现时参考本文档的数据结构和流程。

---

## 1. 概述

相机系统负责管理玩家视角的全部行为。在怪物猎人 like 的战斗中，相机是手感的一半——视角跟丢了怪物，战斗就崩了。

**核心目标**：

- **自由环绕**：右摇杆控制相机围绕角色旋转，无操作时自动回正
- **锁定追踪**：锁定怪物后相机自动跟踪目标，角色移动方向相对于锁定目标
- **平滑过渡**：状态切换无跳变，Lerp 过渡
- **碰撞处理**：墙壁遮挡时不穿模，自动拉近或偏移
- **战斗适配**：攻击/翻滚时相机有微调（FOV 变化、轻微震动）
- **可配置**：每个参数都可在编辑器中调整，不同武器可有不同配置

---

## 2. 核心概念

### 2.1 相机状态机

相机永远处于以下状态之一：

```
                    [自由视角]
                    /        \
         锁定目标    |          | 取消锁定
                    v          v
               [锁定视角]    [自由视角]
                    |
         播放动画/技能
                    |
                    v
               [动作视角]（临时，动画结束自动恢复）
```

| 状态 | 触发条件 | 行为 |
|------|---------|------|
| FreeCamera | 默认 | 右摇杆环绕角色，不操作时缓慢回正到角色背后 |
| LockedCamera | 按下锁定键，有有效目标 | 相机自动跟踪锁定目标，视角被约束在目标方向 |
| ActionCamera | 播放攻击/技能动画 | 临时 FOV 变化 + 轻微偏移，强化打击感，动画结束后自动恢复到前一状态 |

### 2.2 焦点与目标

| 概念 | 类型 | 说明 |
|------|------|------|
| FollowTarget | AActor* | 相机跟随的 Actor（通常是玩家角色） |
| FocusTarget | UFoucusPointComponent* | 锁定目标上的焦点位置（通常是大体型怪物的身体中心偏上） |
| LookAtPoint | FVector | 相机实际注视的世界坐标（自由视角 = 角色前方；锁定视角 = FocusTarget 位置） |

---

## 3. 架构

### 3.1 分层

```
输入层
  Right Stick（Look） / Lock Button（锁定键）
        │
        ▼
PlayerController 层
  AHunterPlayerController
  职责：
    · 接收右摇杆输入 → 传递为 CameraInput (Pitch/Yaw)
    · 接收锁定键 → 触发 LockOn / LockOff
        │
        ▼
CameraManager 层
  AHunterCameraManager : APlayerCameraManager
  职责：
    · 接管 UpdateCamera（不调用 Super，全部自定义）
    · 管理相机状态（Free / Locked / Action）
    · 每帧计算相机位置 + 旋转
    · 执行碰撞检测 + 平滑回退
    · 处理状态切换的插值过渡
        │
        ▼
数据层
  DH CameraData (DataAsset)
    · 每种状态的参数集合
    · 不同武器可有不同 DataAsset
```

### 3.2 `AHunterCameraManager` 职责

这是系统的唯一中枢。UE 的 `APlayerCameraManager` 在每帧末尾调用 `UpdateCamera`，我们在此计算最终的 `POV`（Point of View）。

```
UpdateCamera(DeltaTime):
  │
  ├─ 1. 更新输入（从 PlayerController 读取 CameraInput）
  ├─ 2. 更新状态（检查锁定目标是否有效）
  ├─ 3. 根据当前状态计算理想相机位置
  │      ├─ FreeCamera：球坐标（Pitch, Yaw, Distance）+ 平滑回正
  │      ├─ LockedCamera：在角色与目标之间插值 LookAt，保持俯角
  │      └─ ActionCamera：在 Free/Locked 基础上叠加 FOV 和偏移
  ├─ 4. 碰撞检测：从目标位置向理想位置做射线检测
  │      └─ 若被遮挡 → 拉近距离直到不穿模
  ├─ 5. 平滑插值：当前 POV → 理想 POV（按 LagSpeed）
  └─ 6. 写入 FMinimalViewInfo → 返回
```

### 3.3 `UFoucusPointComponent` 职责

挂在怪物 Actor 上，标记"相机应该看哪里"。

```
UFoucusPointComponent : UActorComponent
  职责：
    · 提供世界空间位置（通常是怪物胸口/头顶偏上）
    · 提供碰撞盒范围（用于相机遮挡检测时知道怪物尺寸）
  
  配置：
    · LocalOffset: FVector（相对于怪物 Root 的偏移）
    · Radius: float（怪物大致半径，用于遮挡检测宽容度）
```

### 3.4 为什么不直接用 SpringArm

UE 的 `USpringArmComponent` 提供了碰撞检测 + 旋转滞后，但它不适合这个系统：

| 需求 | SpringArm | 自定义 CameraManager |
|------|-----------|---------------------|
| 锁定目标跟踪 | 不支持 | 原生支持 |
| 多种相机状态 | 需额外逻辑 | 状态机内置 |
| FOV 动态变化 | 需额外逻辑 | 原生支持 |
| 锁定目标丢失的回退 | 不支持 | 原生支持 |
| 动作相机偏移 | 不支持 | 原生支持 |

结论：用 `APlayerCameraManager` 的 `UpdateCamera` 全覆盖自定义逻辑。

---

## 4. 详细设计

### 4.1 相机数据资产（DataAsset）

```cpp
// 每种相机状态的参数集合
USTRUCT(BlueprintType)
struct FDHFreeCameraParams
{
    // 基础偏移：相机相对于 FollowTarget 的位置（角色本地空间）
    UPROPERTY(EditAnywhere)
    FVector BaseOffset = FVector(0.f, 0.f, 180.f);  // 肩膀高度

    // 基础距离（弹簧臂长度）
    UPROPERTY(EditAnywhere)
    float BaseDistance = 400.f;

    // 碰撞检测半径
    UPROPERTY(EditAnywhere)
    float CollisionRadius = 10.f;

    // 最小距离（被遮挡时最多拉近到此）
    UPROPERTY(EditAnywhere)
    float MinDistance = 100.f;

    // 最大距离（右摇杆推远）
    UPROPERTY(EditAnywhere)
    float MaxDistance = 600.f;

    // 俯仰角范围
    UPROPERTY(EditAnywhere)
    FVector2D PitchRange = FVector2D(-60.f, 30.f);

    // 视角移动灵敏度
    UPROPERTY(EditAnywhere)
    float RotationSpeed = 1.f;

    // 滞后速度（0 = 瞬时，值越大越滞后，类似弹簧效果）
    UPROPERTY(EditAnywhere)
    float LagSpeed = 8.f;

    // 回正速度（不操作摇杆时，Yaw 回到角色背后的速度）
    UPROPERTY(EditAnywhere)
    float AutoCenterSpeed = 2.f;

    // 回正延迟（秒，不操作摇杆多长时间后开始回正）
    UPROPERTY(EditAnywhere)
    float AutoCenterDelay = 1.5f;

    // 默认 FOV
    UPROPERTY(EditAnywhere)
    float DefaultFOV = 90.f;
};

USTRUCT(BlueprintType)
struct FDHLockedCameraParams
{
    // 相对于锁定目标的偏移（升高注视点，避免看脚）
    UPROPERTY(EditAnywhere)
    FVector FocusOffset = FVector(0.f, 0.f, 150.f);

    // 角色到目标的连线方向上的相机偏移（向后拉，让角色和目标都在画面内）
    UPROPERTY(EditAnywhere)
    float CameraBackOffset = 200.f;

    // 相机高度偏移（俯视角度）
    UPROPERTY(EditAnywhere)
    float CameraHeightOffset = 100.f;

    // 锁定时的 FOV（略窄，放大目标）
    UPROPERTY(EditAnywhere)
    float LockedFOV = 80.f;

    // 锁定状态下右摇杆左右推动 → 切换锁定部位（头/尾/身体）
    UPROPERTY(EditAnywhere)
    bool bAllowPartSwitch = true;

    // 锁定切换灵敏度（右摇杆推多少角度算一次切换）
    UPROPERTY(EditAnywhere)
    float PartSwitchThreshold = 0.7f;

    // 锁定丢失后的回退延迟
    UPROPERTY(EditAnywhere)
    float LockLostGracePeriod = 0.5f;

    // 锁定状态的滞后速度
    UPROPERTY(EditAnywhere)
    float LagSpeed = 10.f;
};

USTRUCT(BlueprintType)
struct FDHActionCameraParams
{
    // 攻击期间的 FOV 变化（+ = 拉远，- = 推近）
    UPROPERTY(EditAnywhere)
    float FOVDelta = -5.f;

    // FOV 变化过渡速度
    UPROPERTY(EditAnywhere)
    float FOVBlendSpeed = 20.f;

    // 攻击期间的相机偏移（角色本地空间，如轻微拉近）
    UPROPERTY(EditAnywhere)
    FVector OffsetDelta = FVector(-50.f, 0.f, -20.f);

    // 偏移过渡速度
    UPROPERTY(EditAnywhere)
    float OffsetBlendSpeed = 15.f;
};

// 总配置资产
UCLASS(BlueprintType)
class UDHCameraConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category="Free")
    FDHFreeCameraParams FreeCamera;

    UPROPERTY(EditAnywhere, Category="Locked")
    FDHLockedCameraParams LockedCamera;

    UPROPERTY(EditAnywhere, Category="Action")
    FDHActionCameraParams ActionCamera;

    // 状态切换的混合时间
    UPROPERTY(EditAnywhere, Category="Transition")
    float StateBlendTime = 0.3f;
};
```

不同武器可有不同 Config：`DA_Camera_Default`（全局默认）、`DA_Camera_GreatSword`（大剑更近更窄）等。

---

### 4.2 自由视角（FreeCamera）计算

```
UpdateCamera_Free(DeltaTime):
  │
  ├─ 1. 读取 CameraInput.Yaw / CameraInput.Pitch
  │    ├─ 有输入 → 累积 TargetYaw += Input.Yaw * RotationSpeed
  │    │           TargetPitch += Input.Pitch * RotationSpeed
  │    │           AutoCenterTimer = 0
  │    └─ 无输入 → AutoCenterTimer += DeltaTime
  │         └─ AutoCenterTimer >= AutoCenterDelay
  │              → TargetYaw = FollowTarget.Rotation.Yaw（回正）
  │
  ├─ 2. Clamp Pitch ∈ [PitchRange.X, PitchRange.Y]
  │
  ├─ 3. 球坐标 → 理想相机位置
  │      IdealLocation = FollowTarget.Location
  │                     + BaseOffset
  │                     + RotateBy(TargetPitch, TargetYaw) * FVector(-Distance, 0, 0)
  │
  ├─ 4. 碰撞检测（见 §4.4）
  │
  ├─ 5. FOV = DefaultFOV（若有 ActionCamera 叠加则见 §4.7）
  │
  └─ 6. 平滑插值到理想值
```

### 4.3 锁定视角（LockedCamera）计算

```
UpdateCamera_Locked(DeltaTime):
  │
  ├─ 1. 验证 FocusTarget 是否有效
  │    └─ 无效 → LockLostTimer += DeltaTime
  │         └─ LockLostTimer >= LockLostGracePeriod → 切回 FreeCamera
  │
  ├─ 2. 计算 LookAt 方向：
  │      LockedLookAt = FocusTarget.Location + FocusOffset
  │      PlayerToTarget = (LockedLookAt - FollowTarget.Location).GetSafeNormal()
  │
  ├─ 3. 目标 Yaw = PlayerToTarget 方向（相机始终面对目标）
  │      TargetPitch = -15°（固定俯角，保证视角稳定）
  │
  ├─ 4. 理想相机位置：
  │      MidPoint = Lerp(FollowTarget, LockedLookAt, 0.3)  ← 画面构图中点偏向角色
  │      IdealLocation = MidPoint
  │                     + (-PlayerToTarget * CameraBackOffset)  ← 向后拉
  │                     + UpVector * CameraHeightOffset          ← 抬高
  │
  ├─ 5. 右摇杆左右 → 切换锁定部位
  │      若 bAllowPartSwitch ∧ |Input.Yaw| > PartSwitchThreshold
  │        → 遍历 FocusTarget.Owner 的子 FocusPointComponent
  │        → 切换到相邻的（如头 → 身体 → 尾）
  │
  ├─ 6. 碰撞检测
  ├─ 7. FOV = LockedFOV
  └─ 8. 平滑插值
```

### 4.4 碰撞检测

```
DoCollisionTest(IdealLocation, FollowTarget):
  │
  ├─ 从 FollowTarget.Location 向 IdealLocation 做 SphereTrace
  │    Radius = CollisionRadius
  │    IgnoreActors = { FollowTarget, FocusTarget.Owner }
  │
  ├─ 若 Hit:
  │    ClampedLocation = Hit.Location + Hit.Normal * CollisionRadius  ← 贴墙
  │    ClampedDistance = |ClampedLocation - FollowTarget| - Offset
  │    └─ if ClampedDistance < MinDistance → ClampedDistance = MinDistance
  │
  └─ 返回 ClampedLocation
```

**关键细节**：Ignore 锁定目标自身，否则大怪物的碰撞体会把相机推开。

### 4.5 平滑插值

所有相机计算使用以下平滑策略，避免抖动：

```
CurrentLocation = FMath::VInterpTo(CurrentLocation, IdealLocation, DeltaTime, LagSpeed);
CurrentRotation = FMath::RInterpTo(CurrentRotation, IdealRotation, DeltaTime, LagSpeed);
CurrentFOV      = FMath::FInterpTo(CurrentFOV,      TargetFOV,      DeltaTime, FOVBlendSpeed);
```

LagSpeed 越大 = 越"粘滞"；LagSpeed 越小 = 越"硬"。

### 4.6 锁定目标选择

```
OnLockButtonPressed():
  │
  ├─ 从屏幕中心做射线检测（或使用球形范围扫描）
  │    ├─ 检测类型：SphereOverlapActors
  │    ├─ 半径：可配置（如 1500.f = 15 米）
  │    └─ 过滤：只检测实现了 Lockable 接口的 Actor（怪物）
  │
  ├─ 筛选候选：
  │    ├─ 必须在屏幕上可见（ProjectWorldToScreen，在视口内）
  │    └─ 排除屏幕边缘（距离屏幕中心超过一定角度）
  │
  ├─ 评分排序：
  │    ├─ 距离屏幕中心越近分越高
  │    ├─ 距离玩家越近分越高
  │    └─ 选择得分最高的目标
  │
  └─ SetFocusPoint(Target.FocusComponent)
       → CameraState = LockedCamera
```

**取消锁定**：再次按锁定键或目标死亡/超出范围。

### 4.7 动作相机（ActionCamera）

攻击/技能播放期间临时修改 FOV 和偏移：

```
EnterActionCamera():
  │
  ├─ 记录 PreActionState（Free 或 Locked）
  ├─ TargetFOV  = PreviousFOV + ActionCamera.FOVDelta
  └─ TargetOffset = PreviousOffset + ActionCamera.OffsetDelta

ExitActionCamera():
  │
  └─ 恢复到 PreActionState 及其参数（FOV + Offset Blend 回原值）
```

触发方式：AnimNotify 在 Montage 的开始和结束分别调用 Enter/Exit。

### 4.8 相机震动

```
PlayCameraShake(ShakeClass, Scale):
  │
  └─ ClientStartCameraShake(ShakeClass, Scale)
```

由战斗系统在命中/被命中时调用，CameraManager 仅提供接口。

---

## 5. 数据流

```
[Gamepad Right Stick]                       [Lock Button]
        │                                         │
        ▼                                         ▼
AHunterPlayerController                   AHunterPlayerController
  InputAction_IA_Look                       InputAction_IA_LockOn
  → CameraInput.Yaw/Pitch                   → RequestLockOn()
        │                                         │
        ▼                                         ▼
AHunterCameraManager                      AHunterCameraManager
  CacheCameraInput()                        TryLockOnTarget()
        │                                         │
        ▼                                         ▼
  UpdateCamera(DeltaTime)  ────────────────────────┘
        │
        │  根据 CameraState 选择计算路径
        │
        ├──→ FreeCamera    → 球坐标 + 回正
        ├──→ LockedCamera  → 目标跟踪 + 锁定点切换
        └──→ ActionCamera  → 继承前状态 + FOV/Offset 叠加
        │
        ▼
  DoCollisionTest()  → 射线检测，确保不穿模
        │
        ▼
  SmoothInterpolate()  → Lerp 到目标值
        │
        ▼
  FillViewInfo()  → 写入 FMinimalViewInfo → 引擎渲染
```

---

## 6. 交互时序

### 6.1 锁定目标 → 攻击 → 取消锁定

```
帧 1-30:   FreeCamera，玩家操控右摇杆环视
帧 31:     按下锁定键 → TryLockOnTarget() → 找到前方怪物
帧 32-35:   StateBlend 过渡（0.3s）：FOV 从 90 → 80，相机滑向锁定位置
帧 36-60:   LockedCamera，相机自动跟踪怪物
帧 45:     按下攻击键 → 播放 Montage
帧 45:      AnimNotify: EnterActionCamera → FOV 75, Offset 推近
帧 55:      AnimNotify: ExitActionCamera → Blend 回 LockedCamera 参数
帧 80:     怪物死亡 → FocusTarget 失效
帧 81-82:   LockLostGracePeriod（0.5s 宽限期）
帧 83:     切回 FreeCamera → StateBlend 过渡
```

### 6.2 碰撞遮挡

```
帧 1:    相机在角色后方 400cm
帧 2:    角色后退靠近墙壁
帧 3:    DoCollisionTest: 射线命中墙壁在 150cm 处
帧 4:    相机距离从 400 → 150（LagSpeed 平滑过渡，约 0.3s 完成）
帧 5-:   相机保持在 150cm（贴墙不穿模）

帧 30:   角色向前走出墙角
帧 31:   射线未命中
帧 32:   相机距离从 150 → 400（LagSpeed 平滑恢复，约 0.5s）
```

---

## 7. 配置示例

### 7.1 默认自由视角

| 参数 | 值 | 说明 |
|------|-----|------|
| BaseOffset | (0, 0, 180) | 相机在肩膀高度 |
| BaseDistance | 400 | 默认 4 米 |
| MinDistance | 100 | 贴墙时最近 1 米 |
| MaxDistance | 600 | 摇杆最多推远到 6 米 |
| PitchRange | (-60, 30) | 俯仰限制 |
| LagSpeed | 8 | 中等粘滞感 |
| AutoCenterSpeed | 2 | 缓慢回正 |
| AutoCenterDelay | 1.5s | 1.5 秒后开始回正 |
| DefaultFOV | 90 | 标准视场角 |

### 7.2 锁定视角

| 参数 | 值 | 说明 |
|------|-----|------|
| FocusOffset | (0, 0, 150) | 注视目标上方 1.5 米 |
| CameraBackOffset | 200 | 后拉 2 米 |
| CameraHeightOffset | 100 | 抬高 1 米（微俯视） |
| LockedFOV | 80 | 略窄，放大目标 |
| bAllowPartSwitch | true | 允许切换部位 |
| PartSwitchThreshold | 0.7 | 摇杆推 70% 触发切换 |
| LockLostGracePeriod | 0.5s | 目标丢失 0.5 秒容忍 |
| LagSpeed | 10 | 锁定状态更跟手 |

### 7.3 动作相机（攻击叠加）

| 参数 | 值 | 说明 |
|------|-----|------|
| FOVDelta | -5 | 攻击时略微推近 |
| FOVBlendSpeed | 20 | 快速过渡 |
| OffsetDelta | (-50, 0, -20) | 轻微拉近 + 降低 |
| OffsetBlendSpeed | 15 | — |

---

## 8. 与输入系统的交互

### 8.1 右摇杆

自由视角和锁定视角使用**同一根右摇杆**，但含义不同：

| 状态 | 水平（Yaw） | 垂直（Pitch） |
|------|-----------|-------------|
| FreeCamera | 环绕角色旋转 | 上下调整俯仰 |
| LockedCamera | 切换锁定部位（推超过阈值） | 调整俯仰（受限） |

### 8.2 锁定键

- 短按：锁定/取消锁定
- 锁定状态下再次短按：取消锁定
- 锁定状态下按右摇杆（R3）：重置锁定目标（先取消再锁定下一个）

### 8.3 模式切换时的行为

```
Combat → Menu（打开菜单）
  │
  └─ 相机保持当前位置不动（冻结输入响应）
       UMHInputSubsystem::OnInputModeChanged → CameraManager 暂停处理 CameraInput

Menu → Combat（关闭菜单）
  │
  └─ 恢复相机输入处理，从当前位置继续
```

---

## 9. 文件清单

### 源码文件

| 文件 | 路径 | 说明 |
|------|------|------|
| `HunterCameraManager.h` | `Source/DemonHunter/Public/PlayerCotroller/` | **重构**：主要逻辑全部在此 |
| `HunterCameraManager.cpp` | `Source/DemonHunter/Private/PlayerCotroller/` | **重构**：UpdateCamera 全自定义 |
| `FoucusPointComponent.h` | `Source/DemonHunter/Public/HunterActor/Component/` | **扩展**：添加配置属性 |
| `FoucusPointComponent.cpp` | `Source/DemonHunter/Private/HunterActor/Component/` | **扩展**：碰撞盒逻辑 |
| `DHCameraConfig.h` | `Source/DemonHunter/Public/Data/` | **新建**：相机参数 DataAsset |
| `HunterPlayerController.h` | `Source/DemonHunter/Public/PlayerCotroller/` | **扩展**：添加锁定/相机输入接口 |
| `HunterPlayerController.cpp` | `Source/DemonHunter/Private/PlayerCotroller/` | **扩展**：实现锁定逻辑 |

### 数据资产（在编辑器中创建）

| 资产 | 路径 | 说明 |
|------|------|------|
| `DA_Camera_Default` | `Content/Data/Camera/` | 默认相机配置 |
| `DA_Camera_GreatSword` | `Content/Data/Camera/` | 大剑武器相机（可选，更近更窄） |

---

## 10. 实现优先级

| 优先级 | 功能 | 说明 |
|--------|------|------|
| **P0** | 自由视角（球坐标 + 回正） | 基础操作 |
| **P0** | 碰撞检测 | 无此功能相机穿模 |
| **P0** | 基础平滑插值 | 无此功能视角抖动 |
| **P1** | 锁定视角（单焦点） | Demo 核心战斗 |
| **P1** | 锁定目标选择（评分排序） | — |
| **P1** | 状态切换过渡 | — |
| **P2** | 锁定部位切换 | — |
| **P2** | 动作相机（FOV + 偏移） | 打击感打磨 |
| **P2** | 相机震动 | — |
| **P3** | 多武器 CameraConfig | — |
| **P3** | 相机混合动画（CameraAnim） | 大招演出 |

### Demo MVP（P0+P1）

- 右摇杆自由旋转 + 自动回正
- 碰撞避免
- 单目标锁定 + 跟踪
- 锁定/取消锁定切换
- 平滑的状态过渡

---

## 11. 变更记录

| 日期 | 版本 | 变更内容 |
|------|------|----------|
| 2026-05-23 | v1.0 | 初稿：架构、状态机、相机计算、碰撞检测、配置方案 |
