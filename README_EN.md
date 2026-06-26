# Ar nosurge DX PC Fix Pack

[中文](README.md) | English | [日本語](README_JP.md)

---

PC fix pack for [Ar Nosurge DX / アルノサージュ ～生まれいずる星へ祈る詩～ DX](https://store.steampowered.com/app/1477490).

## Fixed Issues

| Issue | Plugin | Status |
|-------|--------|--------|
| CPU 100% in 3D scenes | CPUFix.asi / CPUFixLite.asi | ✅ |
| Garbled Japanese text on non-JP Windows | LocaleFix.asi | ✅ |
| Env.exe garbled/empty error | Remake | ✅ |

## CPUFix.asi — CPU 100% Fix

**Full Edition** — configurable with hotkeys:

| Action | Hotkey (Default) |
|--------|------|
| Toggle throttle | Keyboard `F1` |
| Toggle throttle | Gamepad `BACK + R3` |

`CPUFix.ini` is auto-generated on first run (with EN/JP/CN comments):

```ini
[CPUFix]
ThrottleMs=1           ; Throttle delay (1-16ms, default 1, do not change)
EnableOnStart=1        ; Throttle on at startup (1=ON 0=OFF)
Hotkey=F1              ; Keyboard hotkey
GamepadHotkey=BACK+R3  ; Gamepad combo (leave empty to disable)
LogEnabled=0           ; Log file (1=ON 0=OFF)
```

**Lite Edition (CPUFixLite.asi)** — no config, no hotkeys, always on. Only writes `CPUFix.log`.

## LocaleFix.asi — Japanese Encoding Fix

Displays correct Japanese text on ANY system locale (Chinese, English, Korean, Russian, etc.). No configuration needed.

## Technical Reports (Chinese)

- [Phase 1 Encoding Fix](src/Phase_1_REPORT_v2.md)
- [Phase 2 CPU Fix](src/Phase_2_REPORT_Final.md)

## Acknowledgments

- [MinHook](https://github.com/TsudaKageyu/minhook)
- [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader)
- DeepSeek — The existence that lends people wings
