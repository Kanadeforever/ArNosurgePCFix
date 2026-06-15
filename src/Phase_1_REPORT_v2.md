# Phase 1 — 日文编码修复 — v2 发行版

> 版本：v2 Release | 日期：2026-06-15  
> 目标：ArnosurgeDX (PC, 64-bit)  
> 状态：✅ 发行版 — 菜单/对话/标题已修复

---

## 1. 修复覆盖

| 场景 | 状态 |
|------|------|
| 菜单文字 | ✅ |
| 对话/说明文字 | ✅ |
| 窗口标题栏 | ✅ |
| 弹窗/对话框 | ✅ 已 Hook，未遇到实例 |
| 下拉列表/选择框 | ✅ 已 Hook，未遇到实例 |

## 2. 已实施 Hook (9 个)

| # | 函数 | DLL | 行为 |
|---|------|-----|------|
| 1 | `GetACP` | kernel32 | 游戏调用→932 |
| 2 | `GetOEMCP` | kernel32 | 同上 |
| 3 | `GetCPInfo` | kernel32 | 游戏调用→查CP932 |
| 4 | `MultiByteToWideChar` | kernel32 | 游戏+非UTF→CP932 |
| 5 | `WideCharToMultiByte` | kernel32 | 同上 |
| 6 | `CreateWindowExA` | user32 | 类名+标题转Unicode |
| 7 | `SetWindowTextA` | user32 | 标题转Unicode |
| 8 | `MessageBoxA` | user32 | 文字+标题转Unicode |
| 9 | `SendMessageA` | user32 | 15种字符串消息处理 |

**安全机制**：`IsGameCaller()` 调用者过滤 + `IS_INTRESOURCE` 保护 + NULL 检查

## 3. 未覆盖场景（未来扩展参考）

### 3.1 GDI 文字绘制

| 函数 | 用途 | 症状(未Hook时) | 触发场景 |
|------|------|----------------|----------|
| `DrawTextExA` | GDI 格式化文字绘制 | HUD/浮层文字乱码 | 游戏用 GDI 绘制 UI |
| `TextOutA` | GDI 简单文字输出 | 同上 | 同上 |
| `ExtTextOutA` | GDI 高级文字输出 | 同上 | 同上 |

> 如果游戏 HUD 使用 D3D11 渲染纹理文字（现代引擎常见），这些不会被触发。

### 3.2 字体选择

| 函数 | 用途 | 症状(未Hook时) | 触发场景 |
|------|------|----------------|----------|
| `CreateFontA` | 创建逻辑字体 | 选中中文字体→日文字形缺失→方框 | 系统无日文字体时 |
| `CreateFontIndirectA` | 同上(LOGFONT结构) | 同上 | 同上 |
| `CreateFontIndirectExA` | 同上(扩展结构) | 同上 | 同上 |

> 当前未触发原因：代码页重定向后，系统可能已自动选择日文兼容字体；Windows 10+ 全局字体回退机制也可能兜底。

### 3.3 DBCS 字符串遍历

| 函数 | 用途 | 症状(未Hook时) | 触发场景 |
|------|------|----------------|----------|
| `CharPrevExA` | 向前找字符边界 | 光标定位错位/越界 | 文本编辑器控件 |
| `CharNextExA` | 向后找字符边界 | 选择文本范围错乱 | 同上 |
| `IsDBCSLeadByteEx` | 判断是否双字节前导 | 同上 | 同上 |

> CP936 和 CP932 的 lead byte 范围不同。如果游戏用这些函数遍历 ANSI 字符串，会错误识别字节边界。

### 3.4 文件 I/O

| 函数 | 用途 | 症状(未Hook时) | 触发场景 |
|------|------|----------------|----------|
| `CreateFileA` | 创建/打开文件 | 日文文件名无法访问 | 存档/截图含日文路径 |
| `CreateDirectoryA` | 创建目录 | 同上 | 同上 |

> 如果游戏路径不含日文字符，不会触发。多数日厂游戏用英文路径名。

### 3.5 对话框资源

| 函数 | 用途 | 症状(未Hook时) | 触发场景 |
|------|------|----------------|----------|
| `CreateDialogIndirectParamA` | 内存模板创建对话框 | 对话框文字乱码 | 自定义对话框 |
| `DialogBoxParamA` | 资源模板对话框 | 同上 | 同上 |

> 对话框模板内嵌的 ANSI 字符串会被 `GetACP→932` 间接修正。但如果模板是编译时嵌入的字节数组而非运行时字符串，Hook `GetACP` 可能不够。

### 3.6 其他

| 函数 | 用途 | 症状(未Hook时) | 触发场景 |
|------|------|----------------|----------|
| `GetWindowTextA` | 读取窗口文本 | 读取到乱码 | 获取其他窗口标题 |
| `GetLocaleInfoA` | 区域信息 | 日期/货币格式异常 | 显示日期时间 |
| `GetTimeZoneInformation` | 时区信息 | 时区名乱码 | 同上 |
| `DirectSoundEnumerateA` | 音频设备枚举 | 设备名乱码 | 音频设置界面 |
| `ImmGetCompositionStringA` | IME 输入组合 | 输入日文时乱码 | 需要输入日文时 |
| `GetClipboardData`/`SetClipboardData` | 剪贴板 | 复制粘贴乱码 | 复制粘贴操作 |

## 4. 崩溃根因回顾

| 尝试 | 结果 | 根因 |
|------|------|------|
| 19 Hook 全量 (v0) | 启动崩溃 | 全局 Hook 影响系统 + `IS_INTRESOURCE` 缺失 |
| 5 Hook 无过滤 | 启动卡死 | 系统中文路径被 CP932 强制转换失败 |
| **5 Hook + IsGameCaller** | **✅** | |
| **+窗口标题/弹窗/列表框** | **✅ v2 发行版** | |

## 5. 产物

```
workspace/archive/phase1-v1/     ← Tier1 + 窗口标题
workspace/archive/phase1-v2/     ← 当前发行版 (9 Hook)
  ├── LocaleFix.asi
  ├── hooks.cpp
  ├── hooks.h
  └── dllmain.cpp
```

## 6. 扩展指南

如需添加新 Hook，模板：

```cpp
// hooks.cpp: 添加原始函数指针
static FN_TYPE (WINAPI *OriginalXxx)(...) = NULL;

// hooks.cpp: 添加 Hook 实现
static RETURN_TYPE WINAPI HookXxx(PARAMS) {
    if (!IsGameCaller()) return OriginalXxx(...);
    // 转换逻辑
}

// hooks.cpp ApplyHooks(): 注册
MH_CreateHookApi(L"dllname.dll", "Xxx", HookXxx, (LPVOID*)&OriginalXxx);
```

注意：如函数参数可能为 `IS_INTRESOURCE`（窗口类名等），必须加判断保护。
