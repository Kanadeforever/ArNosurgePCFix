# Phase 2 — CPU 100% BUG 修复技术报告

> 生成时间：2026-06-15  
> 目标游戏：ArnosurgeDX (PC, 64-bit, D3D11)  
> 作者：Project Guardian Protocol v3.0 + Claude Code  

---

## 1. 问题描述

ArnosurgeDX 在 3D 场景下 CPU 使用率 100%（所有核心满载），2D 菜单恢复正常。无论多强大的 CPU 均出现此问题。

**异常行为**：
- 4 核及以上 → 全核心 100%
- 2 核 → 全核心 50%（各 100%）
- `start /affinity 0x1` 限制单核 → 个位数帧率
- Process Lasso 限制单核 → 正常 60fps（治标不治本）

## 2. 根因分析

### 2.1 排除的假说

| 假说 | 测试方法 | 结论 |
|------|----------|------|
| Present(0,0) 无 vsync | Hook IDXGISwapChain::Present | 游戏中 `SyncInterval=1`，vsync 已开启 |
| D3D11 驱动内部线程 | D3D11_CREATE_DEVICE_PREVENT_INTERNAL_THREADING_OPTIMIZATIONS | 添加标志后游戏死锁 |
| D3D11_SINGLETHREADED | 同上 | 死锁（游戏多线程调用 D3D11） |
| OpenMP 自旋可通过环境变量解决 | OMP_WAIT_POLICY=PASSIVE | 无效（OpenMP 运行时不遵守？或忙等不在 OpenMP 层） |

### 2.2 最终根因

**OpenMP worker 线程在同步屏障（barrier）处忙等（busy-wait），消耗 100% CPU。**

- 游戏使用 `vcomp140.dll`（MSVC OpenMP 运行时）
- 3D 场景中 `#pragma omp parallel for` 大量并行工作
- Worker 线程在 `_vcomp_barrier` 内部通过 `Sleep(0)`/`SwitchToThread()` 忙等
- 2D 菜单中 OpenMP 不活跃 → CPU 正常

证据链：
1. 游戏导入 `vcomp140.dll` → 使用 OpenMP
2. CPU 满载核心数 = `core_count - 1`（主线程 + worker 线程）
3. Hook `Sleep(0)`/`SwitchToThread()`/`WaitForSingleObject(h,0)` 后 CPU 下降
4. 仅 Hook `_vcomp_barrier` 调用者（OMP 线程）即可解决问题

## 3. 技术方案

### 3.1 核心原理

Hook 三个 Windows 忙等原语，为 worker 线程注入最小延迟：

```
Sleep(0)               → Sleep(1)        (0 → 1ms 强制上下文切换)
SwitchToThread()       → Sleep(1)        (主动让出 → 实际等待)
WaitForSingleObject(h,0) → WaitForSingleObject(h,1)  (零超时 → 1ms 超时)
```

`timeBeginPeriod(1)` 将系统定时器精度从 15.6ms 提升至 1ms，确保 `Sleep(1)` ≈ 1ms。

### 3.2 精准节流

早期版本（v5-v14）对所有非主线程节流，导致 2D 菜单卡顿——音频、Steam overlay、输入轮询等常驻线程也被误伤。

**v16 最终方案**：Hook `_vcomp_barrier`（vcomp140.dll 导出函数）标记调用线程为"OpenMP 工作线程"。节流仅对标记过的线程生效。

```
主线程        → 永远放行
OpenMP worker → 已标记 → 节流
音频线程      → 未标记 → 放行
Steam 线程    → 未标记 → 放行
```

### 3.3 架构

```
ArnosurgeDX.exe
  └── dinput8.dll (Ultimate ASI Loader)
        └── CPUFix.asi
              ├── MinHook 初始化
              ├── Hook kernel32!Sleep          → Sleep(0)→Sleep(1)
              ├── Hook kernel32!SwitchToThread → SwitchToThread→Sleep(1)
              ├── Hook kernel32!WaitForSingleObject → 0→1ms
              ├── Hook vcomp140!_vcomp_barrier → 标记 OMP 线程
              ├── RegisterHotKey(F1)           → 键盘切换开关
              ├── XInput 轮询线程              → 手柄切换 (BACK+R3)
              └── cpufix.ini 配置载入
```

### 3.4 迭代历史

| 版本 | 策略 | 结果 |
|------|------|------|
| v2-v4 | Hook DXGI/D3D11 Present/Factory | 无效 (vsync 本就开着) |
| v5 | 三件套 Hook 全部 worker 线程 | 3D OK，2D 卡 |
| v6 | +D3D11_SINGLETHREADED | 死锁 |
| v7 | _vcomp_barrier 加 SwitchToThread | 无效 (忙等在 barrier 内部) |
| v8 | D3D11_PREVENT_INTERNAL_THREADING | 死锁 |
| v9 | Sleep 频控限速 | 无效 (未覆盖 SwitchToThread/Wait) |
| v10 | v5 + 主线程白名单 | 3D 完美，2D 小卡 |
| v11-v14 | 各种频控/自适应阀值 | 2D 误触发 |
| v15 | v10 + 热键 + ini | 3D 完美，2D 手动关 |
| **v16** | **v15 + OpenMP 线程精准标记** | **3D/2D 均完美** |

## 4. 文件清单

```
游戏目录/
├── dinput8.dll        # Ultimate ASI Loader x64
├── CPUFix.asi         # Phase 2 修复插件 (~25KB)
├── cpufix.ini         # 自动生成，可编辑配置
└── cpufix.log         # 运行日志（诊断用）
```

### cpufix.ini 默认配置

```ini
[CPUFix]
ThrottleMs=1        ; 节流延迟 ms (1-16, 默认 1。>1 会导致掉帧)
EnableOnStart=1     ; 启动时节流开关, 1=开 0=关
Hotkey=F1           ; 键盘切换热键
GamepadHotkey=BACK+R3  ; 手柄切换组合键 (XInput), 留空=禁用
```

## 5. 使用说明

1. 将 `CPUFix.asi` 放入游戏目录（与 `ArnosurgeDX.exe` 同级）
2. 确保 `dinput8.dll` (Ultimate ASI Loader) 也在同一目录
3. 启动游戏，节流默认开启
4. 按 `F1` 或手柄 `BACK+R3` 切换开关
5. 首次运行自动生成 `cpufix.ini`，可修改配置

## 6. 技术依赖

- [MinHook](https://github.com/TsudaKageyu/minhook) — API Hook 库 (BSD 许可)
- [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader) — ASI 插件加载器 (MIT 许可)
- Windows `timeBeginPeriod` — 定时器精度提升
- Windows `RegisterHotKey` — 全局热键注册
- XInput — 手柄状态轮询

## 7. 已知限制

- `ThrottleMs` 必须为 1，更大值导致帧率不足 60fps
- 不支持 DirectInput 手柄（仅 XInput）
- 需要 Ultimate ASI Loader（dinput8.dll 代理）
