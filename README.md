# Ar nosurge DX PC 修复插件

中文 | [English](README_EN.md) | [日本語](README_JP.md)

---

PC 版《[Ar Nosurge DX / アルノサージュ ～生まれいずる星へ祈る詩～ DX](https://store.steampowered.com/app/1477490)》修复插件合集。

## 已修复

| 问题 | 插件 | 状态 |
|------|------|------|
| 3D 场景 CPU 100% | CPUFix.asi / CPUFixLite.asi | ✅ |
| 非日文系统日文乱码 | LocaleFix.asi | ✅ |
| Env.exe 乱码/白框报错 | 从零重做一个 | ✅ |

## CPUFix.asi — CPU 100% 修复

**完整版** — 支持热键和配置：

| 操作 | 热键 (默认) |
|------|------|
| 切换节流开关 | 键盘 `F1` |
| 切换节流开关 | 手柄 `BACK + R3` |

首次运行自动生成 `CPUFix.ini`（中/日/英三语注释）：

```ini
[CPUFix]
ThrottleMs=1        ; 节流延迟 (1-16ms, 默认 1, 请勿改动)
EnableOnStart=1     ; 启动时节流开关 (1=开 0=关)
Hotkey=F1           ; 键盘热键
GamepadHotkey=BACK+R3  ; 手柄组合键 (留空禁用)
LogEnabled=0        ; 日志开关 (1=开 0=关)
```

**精简版 (CPUFixLite.asi)** — 无配置、无热键、加载即生效、始终开启。仅输出 `CPUFix.log`。

## LocaleFix.asi — 日文编码修复

在**任何语言系统**（中文/英文/韩文/俄文等）下正确显示日文。无需配置，加载即生效。

## 技术报告

- [Phase 1 编码修复](src/Phase_1_REPORT_v2.md)
- [Phase 2 CPU 修复](src/Phase_2_REPORT_Final.md)

## 依赖

- [MinHook](https://github.com/TsudaKageyu/minhook) — BSD 许可
- [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader) — MIT 许可
