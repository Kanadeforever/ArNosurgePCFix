# Ar nosurge DX PC 修正プラグイン

[中文](README.md) | [English](README_EN.md) | 日本語

---

PC版『アルノサージュDX』修正プラグイン集。

## 修正済み

| 問題 | プラグイン | 状態 |
|------|-----------|------|
| 3DシーンでCPU使用率100% | CPUFix.asi / CPUFixLite.asi | ✅ |
| 非日本語Windowsで文字化け | LocaleFix.asi | ✅ |
| Env.exeの文字化け/白枠 | — | ❌ 断念 |

## CPUFix.asi — CPU使用率100%修正

**フル版** — ホットキーとコンフィグ対応：

| 操作 | ホットキー (デフォルト) |
|------|-----------|
| スロットル切替 | キーボード `F1` |
| スロットル切替 | ゲームパッド `BACK + R3` |

初回起動時に `CPUFix.ini` を自動生成（日中英コメント付き）：

```ini
[CPUFix]
ThrottleMs=1        ; スロットル遅延 (1-16ms, デフォルト1, 変更禁止)
EnableOnStart=1     ; 起動時スロットル (1=ON 0=OFF)
Hotkey=F1           ; キーボードホットキー
GamepadHotkey=BACK+R3  ; ゲームパッドコンボ (空欄で無効)
LogEnabled=0        ; ログ出力 (1=ON 0=OFF)
```

**Lite版 (CPUFixLite.asi)** — 設定不要、ホットキーなし、常時有効。`CPUFix.log` のみ出力。

## LocaleFix.asi — 文字化け修正

あらゆる言語システム（中国語/英語/韓国語/ロシア語等）上で日本語を正しく表示。設定不要。

## 技術レポート（中国語）

- [Phase 1 文字化け修正](src/Phase_1_REPORT_v2.md)
- [Phase 2 CPU修正](src/Phase_2_REPORT_Final.md)

## クレジット

- [MinHook](https://github.com/TsudaKageyu/minhook) — BSD
- [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader) — MIT
