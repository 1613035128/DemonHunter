# 指令输入系统 — 设计方案

> 项目：DemonHunter | 引擎：Unreal Engine 5.7 | 日期：2026-05-23 | 版本：v1.0
>
> 本文档是设计规格，不包含具体代码实现。实现时参考本文档的数据结构和流程。

---

## 1. 概述

指令输入系统（Command Input System）是玩家操作与角色动作之间的中间层。它将即时按键事件转换为**可缓存的指令令牌**，在动画允许的时间窗口内消费令牌并驱动角色动作。

**核心目标**：

- **输入缓冲**：玩家按键时机略早于动画可响应点时，暂存指令，稍后执行
- **连段判定**：在连段窗口内按下特定键，触发派生动作
- **取消规则**：定义哪些动作可以被哪些后续动作中断/覆盖
- **优先级仲裁**：多个缓冲指令竞争时，按预定规则选出最终执行的指令

---

## 2. 核心概念

### 2.1 指令令牌（Command Token）

一个 `FCommandToken` 代表一次玩家输入：

| 字段 | 类型 | 说明 |
|------|------|------|
| CommandID | FName | 指令 ID，如 `Cmd_Attack_Light` |
| Timestamp | float | 按键发生时间（游戏时间） |
| Priority | int32 | 优先级，越大越优先 |
| LifeTime | float | 缓冲有效时长（秒），过期丢弃 |
| SourceInput | UInputAction* | 来源输入动作（调试用） |
| bIsComboInput | bool | 是否在连段窗口内按下 |
| ComboIndex | int32 | 当前连段序号 |

**优先级排序规则**：Priority 降序 → Timestamp 升序（先按的优先）。

---

### 2.2 缓冲窗口（Buffer Window）

缓冲窗口是动画中的一段前缀时间区间。在此期间收到的输入暂存到缓冲队列，等待动画到达**分支点**时消费。

```
Animation Timeline
├──────────┬──────────────────┬──────────┤
│ 禁止输入  │   缓冲窗口        │  分支点   │
│          │  [BufferWindow]  │ (Branch) │
├──────────┼──────────────────┼──────────┤
0s       0.3s               0.7s      1.0s
```

- **禁止输入区**（0~0.3s）：按键丢弃
- **缓冲窗口**（0.3~0.7s）：按键暂存
- **分支点**（0.7s）：消费缓冲队列，选出下一动作

缓冲窗口通过 `UAnimNotifyState_CommandBufferWindow` 在动画 Montage 上标记。

---

### 2.3 分支点与动作表

分支点是动画中的关键时刻。在此刻，系统检查缓冲队列，根据**动作分支表**选出匹配的动作。

#### 动作分支表行结构

| 字段 | 类型 | 说明 |
|------|------|------|
| RowName | FName | 当前动作名，如 `Act_Attack_Light_01` |
| ComboWindowStart | float | 连段窗口起始（归一化时间） |
| ComboWindowEnd | float | 连段窗口结束 |
| ComboRoutes | TArray\<FActionRoute\> | 连段路由（连段窗口内匹配） |
| DefaultRoutes | TArray\<FActionRoute\> | 默认路由（非连段时匹配） |

#### 路由条目

| 字段 | 类型 | 说明 |
|------|------|------|
| InputCommand | FName | 触发的指令 ID |
| ToAction | FName | 转入的动作名 |
| Priority | int32 | 冲突时选最高优先级 |

---

### 2.4 取消规则（Cancel Rule）

定义动作之间能否互相中断：

| 类型 | 行为 |
|------|------|
| None | 不可取消，输入丢弃 |
| SoftCancel | 进入缓冲队列，等到分支点再执行 |
| HardCancel | 立即中断当前动作，直接执行新动作 |

取消规则表行结构：

| 字段 | 类型 | 说明 |
|------|------|------|
| RowName | FName | `{FromAction}_{ByAction}` |
| FromAction | FName | 当前运行中的动作 |
| ByAction | FName | 试图取消它的指令（`"Any"` 为通配） |
| CancelType | EActionCancelType | None / SoftCancel / HardCancel |

**默认行为**：未匹配到规则 → None（丢弃输入）。

---

## 3. 架构

### 3.1 分层

```
EnhancedInput 层
  IA_LightAttack / IA_HeavyAttack / IA_Dodge / IA_Block
        │
        ▼
PlayerController 层
  AHunterPlayerController
  职责：SetupInputComponent 绑定 EnhancedInput Action
        → 回调中转发到 UMHInputSubsystem 广播事件
        │
        ▼
Subsystem 路由层
  UMHInputSubsystem (UGameInstanceSubsystem)
  职责：输入事件广播中心
        · 管理 InputMappingContext 切换（Combat / Menu / UIOnly）
        · 广播 OnInputActionReceived 委托
        · 广播 OnInputModeChanged 委托
        │
        ▼
Component 判定层（核心）
  UCommandInputComponent (UActorComponent)
  挂载在猎人 Character 上
  职责：
        · 缓冲队列管理（Enqueue / Prune / Consume）
        · 取消规则判定
        · 分支点消费（查 DataTable）
        · 连段计数
        · 返回 FName ToAction（不执行动作，纯判定）
        │
        ▼
动画层
  UAnimNotifyState_CommandBufferWindow
  职责：标记缓冲窗口的起止，NotifyEnd 触发分支点消费
```

### 3.2 为什么用 Component 而非 Actor

- Component 挂在 Character 上，生命周期自然绑定
- 天然访问 Owner 的动画实例和状态
- 换角色时自动销毁/重建
- 蓝图中可在 Character 上直接配置

### 3.3 Component 的职责边界

Component **只做判定，不做执行**。它返回下一个动作的名字（`FName ToAction`），由 Character 或 AnimBP 负责实际播放动画、造成伤害等。

这样设计的好处：
- 判定逻辑与表现逻辑解耦
- 未来可接入 GAS（GameplayAbilitySystem），Component 变为 Ability 的输入源
- 方便单元测试（输入 → 输出，无副作用）

---

## 4. 运行时流程

### 4.1 正常按键执行

```
玩家按下攻击键
  │
  ├─ EnhancedInput → AHunterPlayerController::OnLightAttack()
  ├─ UMHInputSubsystem::OnInputActionReceived.Broadcast()
  │
  ├─ UCommandInputComponent::OnInputActionReceived()
  │    ├─ MapInputToCommand() → "Cmd_Attack_Light"
  │    ├─ 创建 Token
  │    ├─ 取消规则判定
  │    │    ├─ 当前 Idle → 无规则 → 直接执行
  │    │    ├─ 当前 Act_A，HardCancel → 立即中断 → 执行
  │    │    ├─ 当前 Act_A，SoftCancel → EnqueueCommand()
  │    │    └─ 当前 Act_A，None → 丢弃
  │    └─ 若直接执行 → 播放动画，记录当前动作
  │
  └─ 动画中 BufferWindow [0.3, 0.7] → NotifyEnd(0.7)
       └─ ConsumeCommandAtBranch() → ToAction → 播放下一动作
```

### 4.2 缓冲执行

```
攻击动画播放中，BufferWindow [0.3, 0.7]
  玩家在 0.5s 按攻击键
    │
    ├─ OnInputActionReceived() → 取消规则: SoftCancel
    └─ EnqueueCommand(Token)   ← 暂存

  动画到 0.7s（分支点）
    │
    └─ NotifyEnd → ConsumeCommandAtBranch("Act_Attack_Light_01")
         ├─ 查找 DataTable → 找到 FActionBranch
         ├─ 匹配 ComboRoutes（若在连段窗口）或 DefaultRoutes
         ├─ 缓冲中有 Cmd_Attack_Light → 命中！
         └─ 返回 ToAction = "Act_Attack_Light_02" → 连段继续
```

### 4.3 优先级仲裁

缓冲队列中多个 Token 竞争时：

1. 过滤：只保留当前分支支持的 CommandID
2. 排序：Priority 降序 → Timestamp 升序
3. 取第一个执行，其余保留（若新动作的取消规则不覆盖则继续保留）

### 4.4 过期清理

每 Tick：
- 遍历缓冲队列，移除 `Now - Token.Timestamp > Token.LifeTime` 的 Token
- 检查连段超时：`Now - LastComboConsumeTime > ComboWindowDuration` → ComboCount 归零

---

## 5. 动作命名规范

### 5.1 动作名

格式：`Act_{Category}_{Variant}_{Index}`

```
Act_Idle                         待机
Act_Walk / Act_Run               移动
Act_Dodge                        闪避/翻滚

Act_Attack_Light_01              轻攻击 第1段
Act_Attack_Light_02              轻攻击 第2段
Act_Attack_Light_03              轻攻击 第3段（终结）
Act_Attack_Heavy_01              重攻击 第1段

Act_Sheathe / Act_Unsheathe      收刀/拔刀
Act_UseItem                      使用道具
Act_Guard                        防御
```

### 5.2 指令 ID

格式：`Cmd_{Category}_{Variant}`

```
Cmd_Attack_Light    Cmd_Attack_Heavy
Cmd_Dodge           Cmd_Sheathe
Cmd_UseItem         Cmd_Guard
Cmd_Sprint          Cmd_Interact
Cmd_Special         Cmd_ChargeAttack
```

---

## 6. 数据资产

系统依赖 3 张 DataTable，在 `Content/Data/` 下创建：

### 6.1 DT_InputCommandMap（指令映射表）

| 行名 | InputAction | CommandID | Priority | LifeTime |
|------|------------|-----------|----------|----------|
| Map_LightAttack | IA_LightAttack | Cmd_Attack_Light | 0 | 0.3 |
| Map_HeavyAttack | IA_HeavyAttack | Cmd_Attack_Heavy | 0 | 0.3 |
| Map_Dodge | IA_Dodge | Cmd_Dodge | 20 | 0.3 |
| Map_Block | IA_Block | Cmd_Guard | 0 | 0.3 |

### 6.2 DT_ActionBranch（动作分支表）

Demo 最简配置——太刀轻攻击三段：

| RowName | ComboWindow | ComboRoutes | DefaultRoutes |
|---------|-------------|-------------|---------------|
| Act_Attack_Light_01 | 0.2~0.7 | Light→02(10), Heavy→Heavy01(15), Dodge→Dodge(20) | Light→01(10), Dodge→Dodge(20) |
| Act_Attack_Light_02 | 0.25~0.65 | Light→03(10), Dodge→Dodge(20) | Dodge→Dodge(20) |
| Act_Attack_Light_03 | 0.3~0.5 | (空) | Dodge→Dodge(20) |

### 6.3 DT_CancelRule（取消规则表）

| RowName | FromAction | ByAction | CancelType |
|---------|-----------|----------|------------|
| Light01_Dodge | Act_Attack_Light_01 | Cmd_Dodge | HardCancel |
| Light02_Dodge | Act_Attack_Light_02 | Cmd_Dodge | HardCancel |
| Light03_Dodge | Act_Attack_Light_03 | Cmd_Dodge | SoftCancel |
| UseItem_Any | Act_UseItem | Any | HardCancel |

---

## 7. 调试支持

控制台命令（开发阶段使用）：

```
MH.Command.ShowBuffer       // 屏幕实时显示缓冲队列
MH.Command.ShowCombo         // 显示连段序号 + 窗口倒计时
MH.Command.DumpBranch <Act>  // 打印指定动作分支表
MH.Command.LogAllInputs      // 开启全部输入日志
```

---

## 8. 大剑蓄力系统（扩展设计）

大剑的蓄力与普通攻击不同：它需要 **Hold 检测**，而非"按键即令牌"。

### 8.1 蓄力状态机

```
[按下 Cmd_ChargeAttack]
        │
   ┌────▼────┐
   │ 蓄力 Lv1 │ (0.0~0.8s) → 松开 → Act_GS_Charge_Lv1
   └────┬────┘
        │ 持续按住 ≥ 0.8s
   ┌────▼────┐
   │ 蓄力 Lv2 │ (0.8~1.6s) → 松开 → Act_GS_Charge_Lv2
   └────┬────┘
        │ 持续按住 ≥ 1.6s
   ┌────▼────┐
   │ 蓄力 Lv3 │ (1.6~2.5s) → 松开 → Act_GS_Charge_Lv3
   └────┬────┘
        │ 超过 2.5s
   ┌────▼────┐
   │ 过蓄释放  │ → Act_GS_Charge_Lv2（惩罚降级）
   └─────────┘

   [任意时刻按 Cmd_Special] → 肩撞中断蓄力
```

### 8.2 肩撞（Tackle）

- **触发**：蓄力中按特殊键（Cmd_Special）
- **霸体**：免疫击飞/击退，伤害减免 50%
- **蓄力跳过**：肩撞后 ComboStage +1，下一次蓄力直接进入更高级

```
纵斩 → 蓄力斩蓄力中
  ├─ [肩撞] → 跳过一层 → 强蓄力斩蓄力
  │    └─ [肩撞] → 再跳一层 → 真・蓄力斩蓄力
  └─ [松开攻击] → 当前等级释放
```

### 8.3 蓄力数据

| 阶段 | 阈值 | 动作 |
|------|------|------|
| Lv1 | 0.0s | Act_GS_Charge_Lv1 |
| Lv2 | 0.8s | Act_GS_Charge_Lv2 |
| Lv3 | 1.6s | Act_GS_Charge_Lv3 |
| 过蓄 | 2.5s | 自动释放 Lv2（惩罚） |

### 8.4 实现建议

蓄力系统独立于指令缓冲系统：
- 不在 `UCommandInputComponent` 中实现
- 建议作为独立 Component（`UChargeAttackComponent`）或 AnimBP 中的状态机
- 与主系统的交互点：松开按键时生成 Token 进入缓冲队列（或立即执行）

---

## 9. 实现优先级

| 优先级 | 功能 | 说明 |
|--------|------|------|
| **P0** | 基础缓冲队列（Enqueue / Consume） | 核心手感 |
| **P0** | 动作分支表（太刀三段轻攻击） | Demo 核心内容 |
| **P0** | AnimNotifyState 缓冲窗口 | 动画师配置入口 |
| **P0** | HardCancel（闪避取消攻击） | 防御手段 |
| **P1** | 优先级仲裁 | 连按时的智能选择 |
| **P1** | 连段窗口（ComboRoutes） | 精准连段判定 |
| **P1** | SoftCancel（终结技后摇取消） | 手感打磨 |
| **P2** | 大剑蓄力系统 | 第二武器 |
| **P2** | 调试 HUD | 开发效率 |
| **P3** | 多武器支持 | 扩展性 |

### Demo MVP（P0 范围）

- 一种武器（太刀），三段轻攻击连段
- 闪避 HardCancel 一切攻击
- 缓冲窗口统一 [0.2, 0.6]
- 先入先出仲裁
- 不做连段窗口判定（缓冲内任何指令走 DefaultRoutes）

---

## 10. 文件清单

### 源码文件

| 文件 | 路径 | 说明 |
|------|------|------|
| `CommandToken.h` | `Source/DemonHunter/Public/Input/` | 令牌结构体（重构现有文件） |
| `ActionBranch.h` | `Source/DemonHunter/Public/Input/` | 分支表 + 路由 + 取消规则结构体 |
| `CommandInputComponent.h/.cpp` | `Source/DemonHunter/Public/Input/` + `Private/Input/` | 核心组件 |
| `MHInputSubsystem.h/.cpp` | `Source/DemonHunter/Public/SubSystems/` + `Private/SubSystems/` | 输入路由子系统 |
| `AnimNotify_CommandBufferWindow.h/.cpp` | `Source/DemonHunter/Public/Animation/` + `Private/Animation/` | 缓冲窗口 Notify |

### 数据资产（在编辑器中创建）

| 资产 | 路径 | 说明 |
|------|------|------|
| `DT_InputCommandMap` | `Content/Data/` | InputAction → CommandID 映射 |
| `DT_ActionBranch` | `Content/Data/` | 动作分支表 |
| `DT_CancelRule` | `Content/Data/` | 取消规则表 |

### 需改造的现有文件

| 文件 | 改动 |
|------|------|
| `HunterPlayerController.h/.cpp` | 输入回调转发到 `UMHInputSubsystem` |
| `InputManager.h/.cpp` | 可废弃或改为调试 Actor |
| `ClientGameInstance.h/.cpp` | 创建时初始化 `UMHInputSubsystem` |

---

## 11. 变更记录

| 日期 | 版本 | 变更内容 |
|------|------|----------|
| 2026-05-23 | v1.0 | 初稿：架构、数据结构、流程、Demo 方案、大剑扩展 |
