# ArnosurgeDX Settings Editor

《アルノサージュ ～生まれいずる星へ祈る詩～ DX》PC 版配置工具。原版 `Setting.exe` 无法在较新的 Windows 系统上运行，此工具提供完整的替代方案。

## 功能

- **图形设置**：分辨率、轮廓线、阴影质量、全屏模式
- **输入设置**：键盘按键绑定（26 个动作）、XInput 手柄按钮映射
- **系统设置**：鼠标操作开关、手柄 A/B 键交换
- **三语界面**：中文 / 日本語 / English，右上角下拉框即时切换
- **按键冲突检测**：同一按键绑定到两个位置时自动清除旧绑定
- **合法按键过滤**：仅接受游戏引擎支持的按键，非法按键自动忽略
- **一键恢复默认**：图形页右下角按钮，恢复游戏原始设置
- **原始 INI 格式**：输出与游戏原生 `Setting.ini` 格式完全一致（无空格、无空行）

## 配置文件

| 项目 | 值 |
|------|-----|
| 路径 | `%USERPROFILE%\Documents\KoeiTecmo\ArnosurgeDX\Setting.ini` |
| 编码 | UTF-8 |
| 格式 | INI |

程序启动时自动读取已有配置，若文件或目录不存在则使用默认值创建。

## 界面说明

### 标签页「主要」（Main）

| 区块 | 内容 |
|------|------|
| 画面设置 | 分辨率（宽×高）、轮廓线（开启/关闭）、阴影（低/中/高/关闭） |
| 窗口模式 | 全屏 复选框 |
| 鼠标控制 | 启用鼠标操作 复选框 |
| 手柄控制 | A/B 键交换（默认/交换） |

右下角「恢复默认设置」按钮，一键将全部配置重置为游戏默认值。

### 标签页「输入」（Input）

| 区块 | 内容 |
|------|------|
| 方向键 | ↑ ↓ ← → 四个方向键绑定 |
| 操作键 | A (确认) / B (取消) / X / Y |
| 肩键 / 功能 | LB / LT / L3 / RB / RT / R3 / START / SELECT |
| 移动 | 角色移动的四方向按键 |
| 镜头 | 镜头控制的四方向按键 |
| 鼠标 | 手柄 XInput 按钮映射（下拉框选择） |

右下角有操作提示。

### 按键绑定操作方法

1. 点击要修改的按键按钮
2. 按钮文字变为 `[请按键...]`
3. 按下键盘上对应的按键
4. 按钮显示捕获到的键名，绑定完成

- 按 Esc 或点击空白处取消绑定
- 非游戏支持的按键会被自动忽略
- 若新按键已被其他动作占用，旧位置自动清除，状态栏显示提示

### 支持的键盘按键

| 类别 | 按键 |
|------|------|
| 功能键 | F1 – F11 |
| 字母 | A – Z |
| 数字 | 1 – 9（对应 KEY_1 – KEY_9） |
| 回车 | Enter |
| 小键盘 | Num 0 – Num 9（对应 KPAD_0 – KPAD_9） |
| 方向键 | ↑ ↓ ← → |

### 支持的手柄按钮（XInput）

A / B / X / Y / L / R / LT / RT / L Stick Button / R Stick Button / View / Menu / ↑ ↓ ← →

## 系统要求

| 项目 | 最低要求 |
|------|---------|
| 操作系统 | Windows 10 / 11（64 位） |
| 其他依赖 | 无（单文件 EXE，无需安装 Python 或任何运行时） |

## 构建

### 环境

- Python 3.13+
- PyInstaller 6.x
- UPX（可选，用于压缩体积）

### 打包命令

```bash
pyinstaller \
    --onefile \
    --windowed \
    --clean \
    --optimize 2 \
    --upx-dir "C:\Software\upx" \
    --name ArnosurgeDX_Env \
    --icon=ArnosurgeDX_Env.ico \
    --add-data "ArnosurgeDX_Env.ico;." \
    arnosurge_settings.py
```

| 参数 | 作用 |
|------|------|
| `--onefile` | 输出单文件 EXE |
| `--windowed` | 无控制台窗口（GUI 应用） |
| `--clean` | 清理构建缓存 |
| `--optimize 2` | Python 字节码优化级别 |
| `--upx-dir` | UPX 路径，压缩体积 30-50% |
| `--icon` | EXE 文件图标 |
| `--add-data` | 内嵌 .ico 文件，用于运行时窗口图标 |

### 项目结构

```
Env/
├── Setting.ini              ← 原始配置文件（参考用）
├── Setting.png              ← 原版设置工具截图（参考用）
├── README.md
└── workspace/
    ├── scripts/
    │   └── arnosurge_settings.py   ← 主程序源码
    └── build/
        └── BUILD_GUIDE.md          ← 打包指南
```

## 技术说明

- **GUI 框架**：Python tkinter + ttk 主题
- **零外部依赖**：仅使用 Python 标准库（`tkinter`, `configparser`, `os`）
- **按键捕获**：通过 tkinter `<Key>` 事件捕获键盘输入，`keysym` → INI 格式映射
- **XInput 映射**：三个 MOUSE_* 字段使用下拉框，选项为 16 个标准 XInput 按钮名
- **INI 读写**：自定义 `save_config()` 函数，不使用 `configparser.write()`，确保输出格式与游戏原版完全一致

## 许可

仅供个人学习与游戏修复用途。ArnosurgeDX 版权归 Koei Tecmo / GUST 所有。
