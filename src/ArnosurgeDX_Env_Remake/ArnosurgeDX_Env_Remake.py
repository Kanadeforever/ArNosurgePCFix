#!/usr/bin/env python3
"""
ArnosurgeDX Settings Editor
Three-language (Chinese / Japanese / English) GUI for editing Setting.ini.
Target config path: %USERPROFILE%\\Documents\\KoeiTecmo\\ArnosurgeDX\\Setting.ini
"""

import os
import sys
import configparser
import tkinter as tk
from tkinter import ttk, messagebox
import tkinter.font as tkfont

# ─── Paths ───────────────────────────────────────────────────────────────────
INI_DIR  = os.path.join(os.environ["USERPROFILE"], "Documents", "KoeiTecmo", "ArnosurgeDX")
INI_PATH = os.path.join(INI_DIR, "Setting.ini")

# ─── Language Dictionaries ───────────────────────────────────────────────────
LANG = {}  # populated after langs defined

LANG_DATA = {
    "zh": {
        "title":         "ArnosurgeDX 设置工具",
        "tab_graphics":  "主要",
        "tab_input":     "输入",
        "group_video":   "画面设置",
        "group_window":  "窗口模式",
        "group_mouse_ctrl": "鼠标控制",
        "group_pad_ctrl":   "手柄控制",
        "graphics_res":  "分辨率",
        "graphics_outline": "轮廓线",
        "graphics_shadow":  "阴影",
        "window_fullscreen": "全屏",
        "mouse_enable":     "启用鼠标操作",
        "pad_abset":        "A/B 键交换",
        # Input action labels
        "action_up":       "上",
        "action_down":     "下",
        "action_left":     "左",
        "action_right":    "右",
        "action_circle":   "A (确认)",
        "action_cross":    "B (取消)",
        "action_square":   "X",
        "action_triangle": "Y",
        "action_l1":       "LB",
        "action_l2":       "LT",
        "action_l3":       "L3",
        "action_r1":       "RB",
        "action_r2":       "RT",
        "action_r3":       "R3",
        "action_start":    "START",
        "action_select":   "SELECT",
        "action_move_up":    "移动：上",
        "action_move_down":  "移动：下",
        "action_move_left":  "移动：左",
        "action_move_right": "移动：右",
        "action_camera_up":    "镜头：上",
        "action_camera_down":  "镜头：下",
        "action_camera_left":  "镜头：左",
        "action_camera_right": "镜头：右",
        "action_mouse_left":   "鼠标左键",
        "action_mouse_right":  "鼠标右键",
        "action_mouse_center": "鼠标中键",
        "btn_ok":     "确定",
        "btn_cancel": "取消",
        "btn_apply":  "应用",
        "btn_bind":   "点击绑定",
        "btn_press_key": "[请按键...]",
        "msg_saved":    "设置已保存。",
        "msg_no_file":  "未找到配置文件，将创建新的。",
        "msg_load_err": "读取配置文件失败",
        "msg_save_err": "保存配置文件失败",
        "lang_label":   "语言",
        # Key binding groups
        "group_direction": "方向键",
        "group_action":    "操作键",
        "group_shoulder":  "肩键 / 功能",
        "group_move":      "移动",
        "group_camera":    "镜头",
        "group_mouse":     "鼠标",
        "input_hint": "操作：点击按钮 → 按下键盘按键完成绑定。非游戏支持的按键将被忽略。",
        # Dropdown display values
        "on_off":    {"0": "关闭", "1": "开启"},
        "shadow_opt": {"0": "低", "1": "中", "2": "高", "3": "关闭"},
        "ab_opt":    {"0": "默认", "1": "交换"},
        "btn_reset": "恢复默认设置",
    },
    "ja": {
        "title":         "アルノサージュDX 設定",
        "tab_graphics":  "メイン",
        "tab_input":     "入力",
        "group_video":   "画面設定",
        "group_window":  "ウィンドウ",
        "group_mouse_ctrl": "マウス操作",
        "group_pad_ctrl":   "パッド設定",
        "graphics_res":  "解像度",
        "graphics_outline": "アウトライン",
        "graphics_shadow":  "影",
        "window_fullscreen": "フルスクリーン",
        "mouse_enable":     "マウス操作",
        "pad_abset":        "A/B ボタン交換",
        "action_up":       "上",
        "action_down":     "下",
        "action_left":     "左",
        "action_right":    "右",
        "action_circle":   "A (決定)",
        "action_cross":    "B (キャンセル)",
        "action_square":   "X",
        "action_triangle": "Y",
        "action_l1":       "LB",
        "action_l2":       "LT",
        "action_l3":       "L3",
        "action_r1":       "RB",
        "action_r2":       "RT",
        "action_r3":       "R3",
        "action_start":    "START",
        "action_select":   "SELECT",
        "action_move_up":    "移動：上",
        "action_move_down":  "移動：下",
        "action_move_left":  "移動：左",
        "action_move_right": "移動：右",
        "action_camera_up":    "カメラ：上",
        "action_camera_down":  "カメラ：下",
        "action_camera_left":  "カメラ：左",
        "action_camera_right": "カメラ：右",
        "action_mouse_left":   "マウス左",
        "action_mouse_right":  "マウス右",
        "action_mouse_center": "マウス中",
        "btn_ok":     "OK",
        "btn_cancel": "キャンセル",
        "btn_apply":  "適用",
        "btn_bind":   "クリックで登録",
        "btn_press_key": "[キーを押して...]",
        "msg_saved":    "設定を保存しました。",
        "msg_no_file":  "設定ファイルがありません。新規作成します。",
        "msg_load_err": "設定の読み込みに失敗",
        "msg_save_err": "設定の保存に失敗",
        "lang_label":   "言語",
        "group_direction": "方向キー",
        "group_action":    "操作キー",
        "group_shoulder":  "ショルダー / 機能",
        "group_move":      "移動",
        "group_camera":    "カメラ",
        "group_mouse":     "マウス",
        "input_hint": "操作：ボタンをクリック → キーを押して登録。ゲーム非対応のキーは無視されます。",
        "on_off":    {"0": "オフ", "1": "オン"},
        "shadow_opt": {"0": "低", "1": "中", "2": "高", "3": "オフ"},
        "ab_opt":    {"0": "標準", "1": "交換"},
        "btn_reset": "デフォルトに戻す",
    },
    "en": {
        "title":         "ArnosurgeDX Settings",
        "tab_graphics":  "Main",
        "tab_input":     "Input",
        "group_video":   "Display",
        "group_window":  "Window Mode",
        "group_mouse_ctrl": "Mouse Control",
        "group_pad_ctrl":   "Controller",
        "graphics_res":  "Resolution",
        "graphics_outline": "Outline",
        "graphics_shadow":  "Shadow",
        "window_fullscreen": "Full Screen",
        "mouse_enable":     "Enable Mouse",
        "pad_abset":        "Swap A/B Buttons",
        "action_up":       "Up",
        "action_down":     "Down",
        "action_left":     "Left",
        "action_right":    "Right",
        "action_circle":   "A (Confirm)",
        "action_cross":    "B (Cancel)",
        "action_square":   "X",
        "action_triangle": "Y",
        "action_l1":       "LB",
        "action_l2":       "LT",
        "action_l3":       "L3",
        "action_r1":       "RB",
        "action_r2":       "RT",
        "action_r3":       "R3",
        "action_start":    "START",
        "action_select":   "SELECT",
        "action_move_up":    "Move: Up",
        "action_move_down":  "Move: Down",
        "action_move_left":  "Move: Left",
        "action_move_right": "Move: Right",
        "action_camera_up":    "Camera: Up",
        "action_camera_down":  "Camera: Down",
        "action_camera_left":  "Camera: Left",
        "action_camera_right": "Camera: Right",
        "action_mouse_left":   "Mouse Left",
        "action_mouse_right":  "Mouse Right",
        "action_mouse_center": "Mouse Center",
        "btn_ok":     "OK",
        "btn_cancel": "Cancel",
        "btn_apply":  "Apply",
        "btn_bind":   "Click to Bind",
        "btn_press_key": "[Press a Key...]",
        "msg_saved":    "Settings saved.",
        "msg_no_file":  "Config file not found; a new one will be created.",
        "msg_load_err": "Failed to load config",
        "msg_save_err": "Failed to save config",
        "lang_label":   "Language",
        "group_direction": "Direction",
        "group_action":    "Action",
        "group_shoulder":  "Shoulder / System",
        "group_move":      "Movement",
        "group_camera":    "Camera",
        "group_mouse":     "Mouse",
        "input_hint": "Click a button → Press a key to bind. Only game-supported keys are accepted.",
        "on_off":    {"0": "OFF", "1": "ON"},
        "shadow_opt": {"0": "Low", "1": "Mid", "2": "High", "3": "OFF"},
        "ab_opt":    {"0": "Default", "1": "Swap"},
        "btn_reset": "Reset to Defaults",
    },
}

# ─── Valid key set & tkinter→INI mapping ─────────────────────────────────────
# Only these INI key names are accepted by the game engine.
# Single letters A..Z are handled separately (tk keysym is lowercase → uppercase).
VALID_KEYS = {
    # F-keys (F1–F11, no F12)
    "F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11",
    # Letters A..Z
    *(chr(c) for c in range(ord("A"), ord("Z") + 1)),
    # Number keys (KEY_1 … KEY_9)
    "KEY_1","KEY_2","KEY_3","KEY_4","KEY_5","KEY_6","KEY_7","KEY_8","KEY_9",
    # Enter
    "ENTER",
    # Numpad 0..9
    "KPAD_0","KPAD_1","KPAD_2","KPAD_3","KPAD_4","KPAD_5","KPAD_6","KPAD_7","KPAD_8","KPAD_9",
    # Arrow keys
    "UP_ARROW","DOWN_ARROW","LEFT_ARROW","RIGHT_ARROW",
}

# tkinter keysym → INI key name (only valid keys)
KEY_MAP_TK_TO_INI = {
    "Up": "UP_ARROW", "Down": "DOWN_ARROW", "Left": "LEFT_ARROW", "Right": "RIGHT_ARROW",
    "Return": "ENTER",
    "F1":"F1","F2":"F2","F3":"F3","F4":"F4","F5":"F5","F6":"F6",
    "F7":"F7","F8":"F8","F9":"F9","F10":"F10","F11":"F11",
    "KP_0":"KPAD_0","KP_1":"KPAD_1","KP_2":"KPAD_2","KP_3":"KPAD_3","KP_4":"KPAD_4",
    "KP_5":"KPAD_5","KP_6":"KPAD_6","KP_7":"KPAD_7","KP_8":"KPAD_8","KP_9":"KPAD_9",
    "1":"KEY_1","2":"KEY_2","3":"KEY_3","4":"KEY_4","5":"KEY_5",
    "6":"KEY_6","7":"KEY_7","8":"KEY_8","9":"KEY_9",
}
# Build dynamic mapping for A..Z (tk keysym is lowercase)
for _c in range(ord("a"), ord("z") + 1):
    _lo = chr(_c)
    _up = chr(_c - 32)
    KEY_MAP_TK_TO_INI[_lo] = _up

# Reverse map
KEY_MAP_INI_TO_TK = {v: k for k, v in KEY_MAP_TK_TO_INI.items()}

# Build dict: ini_key → internal identifier
ACTION_KEYS = [
    ("UP",        "action_up"),
    ("DOWN",      "action_down"),
    ("RIGHT",     "action_right"),
    ("LEFT",      "action_left"),
    ("CIRCLE",    "action_circle"),
    ("CROSS",     "action_cross"),
    ("SQUARE",    "action_square"),
    ("TRIANGLE",  "action_triangle"),
    ("L1",        "action_l1"),
    ("L2",        "action_l2"),
    ("L3",        "action_l3"),
    ("R1",        "action_r1"),
    ("R2",        "action_r2"),
    ("R3",        "action_r3"),
    ("START",     "action_start"),
    ("SELECT",    "action_select"),
    ("MOVE_UP",   "action_move_up"),
    ("MOVE_DOWN", "action_move_down"),
    ("MOVE_LEFT", "action_move_left"),
    ("MOVE_RIGHT","action_move_right"),
    ("CAMERA_UP",   "action_camera_up"),
    ("CAMERA_DOWN", "action_camera_down"),
    ("CAMERA_LEFT", "action_camera_left"),
    ("CAMERA_RIGHT","action_camera_right"),
    ("MOUSE_LEFT",  "action_mouse_left"),
    ("MOUSE_RIGHT", "action_mouse_right"),
    ("MOUSE_CENTER","action_mouse_center"),
]

# Key groups for Input tab layout — (group_lang_key, [ini_keys...])
KEY_GROUPS_LEFT = [
    ("group_direction", ["UP", "DOWN", "LEFT", "RIGHT"]),
    ("group_action",    ["CIRCLE", "CROSS", "SQUARE", "TRIANGLE"]),
    ("group_shoulder",  ["L1", "L2", "L3", "R1", "R2", "R3", "START", "SELECT"]),
]
KEY_GROUPS_RIGHT = [
    ("group_move",   ["MOVE_UP", "MOVE_DOWN", "MOVE_LEFT", "MOVE_RIGHT"]),
    ("group_camera", ["CAMERA_UP", "CAMERA_DOWN", "CAMERA_LEFT", "CAMERA_RIGHT"]),
]

# XInput controller buttons (for MOUSE_LEFT / MOUSE_RIGHT / MOUSE_CENTER)
XINPUT_BUTTONS = [
    "A", "B", "X", "Y",
    "L", "R", "LT", "RT",
    "L Stick Button", "R Stick Button",
    "View", "Menu",
    "UP", "DOWN", "LEFT", "RIGHT",
]
MOUSE_KEYS = ["MOUSE_LEFT", "MOUSE_RIGHT", "MOUSE_CENTER"]

# Lookup: ini_key → action_id
INI_TO_ACTION = {ini: act for ini, act in ACTION_KEYS}

# ─── Config I/O ──────────────────────────────────────────────────────────────
def load_config():
    """Load Setting.ini, return configparser or None."""
    cfg = configparser.ConfigParser()
    cfg.optionxform = str  # preserve case
    if not os.path.exists(INI_PATH):
        return None
    try:
        cfg.read(INI_PATH, encoding="utf-8")
        return cfg
    except Exception as e:
        messagebox.showerror(
            LANG["msg_load_err"],
            f"{LANG['msg_load_err']}:\n{INI_PATH}\n\n{e}"
        )
        return None

def save_config(cfg):
    """Write configparser back to file; create directory if missing.
    Outputs the exact original INI format: no blank lines, no spaces around '='."""
    try:
        os.makedirs(INI_DIR, exist_ok=True)
        lines = []
        for section in cfg.sections():
            lines.append(f"[{section}]")
            for key, val in cfg.items(section):
                lines.append(f"{key}={val}")
        with open(INI_PATH, "w", encoding="utf-8") as f:
            f.write("\n".join(lines) + "\n")
        return True
    except Exception as e:
        messagebox.showerror(
            LANG["msg_save_err"],
            f"{LANG['msg_save_err']}:\n{INI_PATH}\n\n{e}"
        )
        return False

def get_default_config():
    """Return a configparser with sensible defaults."""
    cfg = configparser.ConfigParser()
    cfg.optionxform = str
    cfg["Graphics"] = {
        "ScreenWidth":  "1920",
        "ScreenHeight": "1080",
        "Outline":      "1",
        "Shadow":       "2",
    }
    cfg["Window"] = {"FullScreen": "0"}
    cfg["Mouse"]  = {"MOUSE_ENABLE": "1"}
    cfg["Input"]  = {
        "UP":"UP_ARROW","DOWN":"DOWN_ARROW","RIGHT":"RIGHT_ARROW","LEFT":"LEFT_ARROW",
        "CIRCLE":"L","CROSS":"K","SQUARE":"J","TRIANGLE":"I",
        "L1":"Q","L2":"U","L3":"F","R1":"E","R2":"O","R3":"H",
        "START":"ENTER","SELECT":"N",
        "MOVE_UP":"W","MOVE_DOWN":"S","MOVE_LEFT":"A","MOVE_RIGHT":"D",
        "CAMERA_UP":"T","CAMERA_DOWN":"G","CAMERA_LEFT":"R","CAMERA_RIGHT":"Y",
        "MOUSE_LEFT":"A","MOUSE_RIGHT":"B","MOUSE_CENTER":"X",
    }
    cfg["Pad"] = {"AB_SET": "0"}
    return cfg

# ─── Display-friendly name for a bound key ───────────────────────────────────
def ini_key_to_display(ini_val):
    """Convert INI key value to display label."""
    # Arrow keys → symbols
    ARROW_MAP = {"UP_ARROW": "↑", "DOWN_ARROW": "↓", "LEFT_ARROW": "←", "RIGHT_ARROW": "→"}
    if ini_val in ARROW_MAP:
        return ARROW_MAP[ini_val]
    # KPAD_X → "Num X"
    if ini_val.startswith("KPAD_"):
        return "Num" + ini_val[5:]
    # KEY_X → just the number
    if ini_val.startswith("KEY_"):
        return ini_val[4:]
    # ENTER
    if ini_val == "ENTER":
        return "Enter"
    # F1-F11, single letters
    return ini_val

# ─── Main Application ────────────────────────────────────────────────────────
class SettingsEditor(tk.Tk):
    def __init__(self):
        super().__init__()
        self.lang_code = "zh"
        self._set_lang()
        self._setup_style()

        self.title(LANG["title"])
        self.resizable(False, False)
        self._set_app_icon()
        self.protocol("WM_DELETE_WINDOW", self._on_cancel)

        # Config data
        self.cfg = load_config()
        if self.cfg is None:
            self.cfg = get_default_config()
            # Only show info if file truly missing; not an error
            if not os.path.exists(INI_PATH):
                print(f"[INFO] {LANG['msg_no_file']}: {INI_PATH}")

        self._build_ui()
        self._load_to_ui()

        # Center on screen
        self.update_idletasks()
        sw = self.winfo_screenwidth()
        sh = self.winfo_screenheight()
        ww = self.winfo_width()
        wh = self.winfo_height()
        x = (sw - ww) // 2
        y = (sh - wh) // 2
        self.geometry(f"+{x}+{y}")

    # ── Icon ─────────────────────────────────────────────────────────────
    def _set_app_icon(self):
        """Set window/taskbar icon. Looks for .ico beside exe or in bundle."""
        icon_names = ["ArnosurgeDX_Env.ico", "icon.ico"]
        search_dirs = []
        if getattr(sys, "frozen", False):
            search_dirs.append(os.path.dirname(sys.executable))
            search_dirs.append(sys._MEIPASS)
        else:
            search_dirs.append(os.path.dirname(os.path.abspath(__file__)))
        search_dirs.append(os.getcwd())

        for d in search_dirs:
            for name in icon_names:
                path = os.path.join(d, name)
                if os.path.isfile(path):
                    try:
                        self.iconbitmap(path)
                    except Exception:
                        pass
                    return

    # ── Language ──────────────────────────────────────────────────────────
    def _set_lang(self):
        global LANG
        LANG = LANG_DATA[self.lang_code]

    # ── Styling ───────────────────────────────────────────────────────────
    def _setup_style(self):
        style = ttk.Style()
        # Try modern themes
        for theme in ("vista", "clam", "alt"):
            if theme in style.theme_names():
                try:
                    style.theme_use(theme)
                    break
                except tk.TclError:
                    continue

        # Global font
        default_font = ("Segoe UI", 9) if "Segoe UI" in tkfont.families() else ("TkDefaultFont", 9)
        self.option_add("*Font", default_font)

        # Custom styles
        style.configure("Title.TLabel", font=(default_font[0], 13, "bold"))
        style.configure("Group.TLabelframe", relief=tk.GROOVE, borderwidth=1)
        style.configure("Group.TLabelframe.Label", font=(default_font[0], 9, "bold"))
        style.configure("Status.TLabel", font=(default_font[0], 8), padding=(4, 1))
        style.configure("TNotebook.Tab", padding=(28, 2, 28, 2), font=(default_font[0], 9))
        style.configure("KeyBtn.TButton", width=13)
        style.configure("ActionBtn.TButton", width=13, padding=(4, 2))

    def _switch_lang(self, event=None):
        self.lang_code = self._lang_var.get()
        self._set_lang()
        self.title(LANG["title"])
        # Rebuild UI
        for w in self.winfo_children():
            w.destroy()
        self._build_ui()
        self._load_to_ui()

    # ── Build UI ──────────────────────────────────────────────────────────
    def _build_ui(self):
        # ---- Top bar: title + language selector ----
        top = ttk.Frame(self)
        top.pack(fill=tk.X, padx=10, pady=(10, 0))

        ttk.Label(top, text=LANG["title"], style="Title.TLabel").pack(side=tk.LEFT)

        lang_frame = ttk.Frame(top)
        lang_frame.pack(side=tk.RIGHT)
        ttk.Label(lang_frame, text=LANG["lang_label"] + ":").pack(side=tk.LEFT, padx=(0, 4))
        self._lang_var = tk.StringVar(value=self.lang_code)
        lang_combo = ttk.Combobox(lang_frame, textvariable=self._lang_var,
                                  values=["zh", "ja", "en"], state="readonly",
                                  width=4, font=("", 10))
        lang_combo.pack(side=tk.LEFT)
        lang_combo.bind("<<ComboboxSelected>>", self._switch_lang)

        # ---- Notebook (tabs) ----
        nb = ttk.Notebook(self)
        nb.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

        self._build_graphics_tab(nb)
        self._build_input_tab(nb)

        # ---- Bottom buttons ----
        btn_frame = ttk.Frame(self)
        btn_frame.pack(fill=tk.X, padx=10, pady=(0, 10))

        ttk.Button(btn_frame, text=LANG["btn_ok"], command=self._on_ok).pack(side=tk.RIGHT, padx=4)
        ttk.Button(btn_frame, text=LANG["btn_cancel"], command=self._on_cancel).pack(side=tk.RIGHT, padx=4)
        ttk.Button(btn_frame, text=LANG["btn_apply"], command=self._on_apply).pack(side=tk.RIGHT, padx=4)

        # Status bar
        self._status_var = tk.StringVar()
        status = ttk.Label(self, textvariable=self._status_var, relief=tk.SUNKEN,
                           anchor=tk.W, style="Status.TLabel")
        status.pack(fill=tk.X, side=tk.BOTTOM)
        self._status_var.set(f"  {INI_PATH}")

    # ── Graphics tab (画面 / 窗口 / 鼠标 / 手柄 合并) ───────────────────
    def _build_graphics_tab(self, nb):
        f = ttk.Frame(nb, padding=(12, 8))
        nb.add(f, text=LANG["tab_graphics"])

        # === Group: 画面设置 ===
        video = ttk.LabelFrame(f, text=LANG["group_video"], padding=(8, 4),
                               style="Group.TLabelframe")
        video.pack(fill=tk.X, pady=(0, 6))

        # Resolution
        row = ttk.Frame(video)
        row.pack(fill=tk.X, pady=4)
        ttk.Label(row, text=LANG["graphics_res"] + ":", width=14, anchor=tk.E).pack(side=tk.LEFT)
        self._res_w_var = tk.StringVar()
        ttk.Combobox(row, textvariable=self._res_w_var, width=6,
                     values=["3840","2560","1920","1600","1366","1280"]).pack(side=tk.LEFT, padx=(4, 2))
        ttk.Label(row, text="×").pack(side=tk.LEFT)
        self._res_h_var = tk.StringVar()
        ttk.Combobox(row, textvariable=self._res_h_var, width=6,
                     values=["2160","1440","1080","900","768","720"]).pack(side=tk.LEFT, padx=2)

        # Outline + Shadow side by side
        props = ttk.Frame(video)
        props.pack(fill=tk.X, pady=1)
        ttk.Label(props, text=LANG["graphics_outline"] + ":", width=14, anchor=tk.E).pack(side=tk.LEFT)
        self._outline_var = tk.StringVar()
        ttk.Combobox(props, textvariable=self._outline_var, width=10,
                     values=list(LANG["on_off"].values()), state="readonly").pack(side=tk.LEFT, padx=4)
        ttk.Label(props, text="  " + LANG["graphics_shadow"] + ":").pack(side=tk.LEFT)
        self._shadow_var = tk.StringVar()
        shadow_display = [LANG["shadow_opt"][str(i)] for i in range(4)]
        ttk.Combobox(props, textvariable=self._shadow_var, width=8,
                     values=shadow_display, state="readonly").pack(side=tk.LEFT, padx=4)

        # === Row: 窗口模式 + 鼠标控制 side by side ===
        row_bottom = ttk.Frame(f)
        row_bottom.pack(fill=tk.X, pady=(0, 6))

        win = ttk.LabelFrame(row_bottom, text=LANG["group_window"], padding=(8, 4),
                             style="Group.TLabelframe")
        win.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(0, 4))
        self._fullscreen_var = tk.BooleanVar()
        ttk.Checkbutton(win, text=LANG["window_fullscreen"],
                        variable=self._fullscreen_var).pack(anchor=tk.W)

        mouse_ctrl = ttk.LabelFrame(row_bottom, text=LANG["group_mouse_ctrl"],
                                    padding=(8, 4), style="Group.TLabelframe")
        mouse_ctrl.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(4, 0))
        self._mouse_var = tk.BooleanVar()
        ttk.Checkbutton(mouse_ctrl, text=LANG["mouse_enable"],
                        variable=self._mouse_var).pack(anchor=tk.W)

        # === Row: 手柄控制 ===
        pad_ctrl = ttk.LabelFrame(f, text=LANG["group_pad_ctrl"],
                                  padding=(8, 4), style="Group.TLabelframe")
        pad_ctrl.pack(fill=tk.X, pady=(0, 6))
        ab_row = ttk.Frame(pad_ctrl)
        ab_row.pack(fill=tk.X)
        ttk.Label(ab_row, text=LANG["pad_abset"] + ":", width=14, anchor=tk.E).pack(side=tk.LEFT, padx=(0, 6))
        self._abset_var = tk.StringVar()
        ttk.Combobox(ab_row, textvariable=self._abset_var, width=10,
                     values=list(LANG["ab_opt"].values()), state="readonly").pack(side=tk.LEFT)

        # Reset button
        ttk.Button(f, text=LANG["btn_reset"], command=self._on_reset_defaults).pack(
            side=tk.BOTTOM, anchor=tk.SE, pady=(6, 0))

    def _on_reset_defaults(self):
        """Reset all config to the game's default values."""
        self.cfg = self._default_config()
        self._load_to_ui()
        self._status_var.set("  ↺ 已恢复默认设置")

    @staticmethod
    def _default_config():
        """Return configparser with game defaults."""
        cfg = configparser.ConfigParser()
        cfg.optionxform = str
        cfg["Graphics"] = {"ScreenWidth":"1280","ScreenHeight":"720","Outline":"1","Shadow":"0"}
        cfg["Window"]  = {"FullScreen":"0"}
        cfg["Mouse"]   = {"MOUSE_ENABLE":"0"}
        cfg["Input"]   = {
            "UP":"UP_ARROW","DOWN":"DOWN_ARROW","RIGHT":"RIGHT_ARROW","LEFT":"LEFT_ARROW",
            "CIRCLE":"L","CROSS":"K","SQUARE":"J","TRIANGLE":"I",
            "L1":"Q","L2":"U","L3":"F","R1":"E","R2":"O","R3":"H",
            "START":"ENTER","SELECT":"N",
            "MOVE_UP":"W","MOVE_DOWN":"S","MOVE_LEFT":"A","MOVE_RIGHT":"D",
            "CAMERA_UP":"T","CAMERA_DOWN":"G","CAMERA_LEFT":"R","CAMERA_RIGHT":"Y",
            "MOUSE_LEFT":"A","MOUSE_RIGHT":"B","MOUSE_CENTER":"X",
        }
        cfg["Pad"] = {"AB_SET":"0"}
        return cfg

    # ── Input tab (key bindings + mouse + pad) ─────────────────────────────
    def _build_input_tab(self, nb):
        outer = ttk.Frame(nb, padding=(12, 10))
        nb.add(outer, text=LANG["tab_input"])

        self._key_bind_widgets = []

        # Two-column container
        left_col = ttk.Frame(outer)
        left_col.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(0, 6))
        right_col = ttk.Frame(outer)
        right_col.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(6, 0))

        # Build groups in left & right columns
        for col_frame, groups in [(left_col, KEY_GROUPS_LEFT), (right_col, KEY_GROUPS_RIGHT)]:
            for group_key, ini_keys in groups:
                g = ttk.LabelFrame(col_frame, text=LANG[group_key], padding=(8, 4),
                                   style="Group.TLabelframe")
                g.pack(fill=tk.X, pady=(0, 8))

                for ini_key in ini_keys:
                    row = ttk.Frame(g)
                    row.pack(fill=tk.X, pady=1)

                    action_id = INI_TO_ACTION[ini_key]
                    ttk.Label(row, text=LANG[action_id], width=14, anchor=tk.E).pack(side=tk.LEFT, padx=(0, 6))

                    btn = tk.Button(row, text="", width=14, relief=tk.FLAT,
                                    command=lambda k=ini_key: self._start_key_capture(k))
                    btn.pack(side=tk.LEFT)

                    self._key_bind_widgets.append((ini_key, btn))

        # Mouse group — XInput dropdowns (below right column groups)
        self._xinput_vars = {}
        mouse_group = ttk.LabelFrame(right_col, text=LANG["group_mouse"],
                                     padding=(8, 4), style="Group.TLabelframe")
        mouse_group.pack(fill=tk.X, pady=(0, 8))

        def make_xinput_handler(changed_key):
            """Return a callback that clears duplicate selections on other dropdowns."""
            def _on_select(event):
                val = self._xinput_vars[changed_key].get()
                if not val:
                    return
                for other_key, var in self._xinput_vars.items():
                    if other_key != changed_key and var.get() == val:
                        var.set("")
                        self._status_var.set(f"  ⚠ {val} 已被占用，已清除另一位置")
                        break
            return _on_select

        for ini_key in MOUSE_KEYS:
            row = ttk.Frame(mouse_group)
            row.pack(fill=tk.X, pady=1)
            action_id = INI_TO_ACTION[ini_key]
            ttk.Label(row, text=LANG[action_id], width=14, anchor=tk.E).pack(side=tk.LEFT, padx=(0, 6))
            var = tk.StringVar()
            combo = ttk.Combobox(row, textvariable=var, values=XINPUT_BUTTONS,
                                 state="readonly", width=15)
            combo.pack(side=tk.LEFT)
            combo.bind("<<ComboboxSelected>>", make_xinput_handler(ini_key))
            self._xinput_vars[ini_key] = var

        # Hint text — positioned right after mouse group, darker color
        hint = ttk.Label(right_col, text=LANG["input_hint"],
                         wraplength=260, justify=tk.LEFT,
                         foreground="#444444", font=("", 8))
        hint.pack(fill=tk.X, pady=(4, 0))

    def _start_key_capture(self, ini_key):
        """Enter key-capture mode for a key binding button."""
        # Release any previous capture first
        if hasattr(self, "_release_capture") and self._release_capture:
            prev = self._release_capture
            self._release_capture = None
            try:
                prev()
            except Exception:
                pass

        # Find the button widget for this ini_key
        for k, btn in self._key_bind_widgets:
            if k == ini_key:
                target_btn = btn
                break
        else:
            return

        old_binding = self._grab_tk_binding(ini_key)
        target_btn.config(text=LANG["btn_press_key"], relief=tk.SUNKEN)
        # Tk path of the target button for identity comparison
        target_path = str(target_btn)

        def on_key(event):
            """Handle key press during capture."""
            try:
                # Skip modifier-only presses
                if event.keysym in ("Shift_L", "Shift_R", "Control_L", "Control_R",
                                    "Alt_L", "Alt_R", "Meta_L", "Meta_R", "Win_L", "Win_R"):
                    return "break"

                # Map tkinter keysym to INI key name
                keysym = event.keysym
                ini_val = KEY_MAP_TK_TO_INI.get(keysym, "")

                # Numpad detection via hardware keycode (Windows: 96-105 = Numpad 0-9)
                # Some systems report numpad keysym as regular digits; keycode is reliable.
                kc = getattr(event, "keycode", 0)
                if 96 <= kc <= 105:
                    ini_val = f"KPAD_{kc - 96}"

                # Letters: tk keysym is lowercase, INI uses uppercase
                if not ini_val and len(keysym) == 1 and keysym.isalpha():
                    ini_val = keysym.upper()

                # Reject keys not in the game's supported set
                if ini_val not in VALID_KEYS:
                    return "break"

                # Ensure Input section exists
                if "Input" not in self.cfg:
                    self.cfg["Input"] = {}

                # Check for duplicate — if the same key is already bound elsewhere, clear it
                dup_cleared = False
                for other_key, btn in self._key_bind_widgets:
                    if other_key != ini_key and other_key not in MOUSE_KEYS:
                        if self.cfg["Input"].get(other_key, "") == ini_val:
                            self.cfg["Input"][other_key] = ""
                            btn.config(text=LANG["btn_bind"])
                            dup_cleared = True
                            break

                self.cfg["Input"][ini_key] = ini_val

                # Update button display and release capture
                target_btn.config(text=ini_key_to_display(ini_val), relief=tk.RAISED)
                if dup_cleared:
                    self._status_var.set(f"  ⚠ {ini_val} 已有绑定，已从原位置清除")
            finally:
                self._release_capture()
            return "break"

        def on_click_outside(event):
            """Cancel capture if user clicks elsewhere."""
            try:
                # Compare by Tk path — "is" doesn't work for ttk widgets
                widget = event.widget
                while widget is not None:
                    if str(widget) == target_path:
                        return  # clicked the same button; stay in capture mode
                    try:
                        widget = widget.master
                    except AttributeError:
                        break
                # Clicked elsewhere — cancel
                target_btn.config(
                    text=ini_key_to_display(old_binding) if old_binding else LANG["btn_bind"],
                    relief=tk.RAISED
                )
            finally:
                self._release_capture()

        def _cleanup():
            try:
                self.unbind("<Key>")
                self.unbind("<Button-1>")
            except Exception:
                pass

        self._release_capture = _cleanup
        self.bind("<Key>", on_key)
        # Defer <Button-1> binding so the current click doesn't trigger cancel
        self.after(100, lambda: self.bind("<Button-1>", on_click_outside))

    def _grab_tk_binding(self, ini_key):
        """Get current ini value for a key binding."""
        if "Input" not in self.cfg:
            return None
        return self.cfg["Input"].get(ini_key, "")

    # ── Pad tab ───────────────────────────────────────────────────────────
    # ── Load config → UI ──────────────────────────────────────────────────
    def _load_to_ui(self):
        # Helper: convert INI value → display text
        def _display(dict_key, ini_val):
            return LANG[dict_key].get(ini_val, ini_val)

        # Graphics
        self._res_w_var.set(self.cfg["Graphics"].get("ScreenWidth", "1280"))
        self._res_h_var.set(self.cfg["Graphics"].get("ScreenHeight", "720"))
        self._outline_var.set(_display("on_off", self.cfg["Graphics"].get("Outline", "1")))
        self._shadow_var.set(_display("shadow_opt", self.cfg["Graphics"].get("Shadow", "0")))

        # Window
        self._fullscreen_var.set(self.cfg["Window"].get("FullScreen", "0") == "1")

        # Mouse
        self._mouse_var.set(self.cfg["Mouse"].get("MOUSE_ENABLE", "1") == "1")

        # Input key bindings
        for ini_key, btn in self._key_bind_widgets:
            val = self.cfg["Input"].get(ini_key, "")
            if val:
                btn.config(text=ini_key_to_display(val))
            else:
                btn.config(text=LANG["btn_bind"])

        # XInput dropdowns (mouse/controller group)
        for ini_key, var in self._xinput_vars.items():
            val = self.cfg["Input"].get(ini_key, "")
            var.set(val if val in XINPUT_BUTTONS else "")

        # Pad
        self._abset_var.set(_display("ab_opt", self.cfg["Pad"].get("AB_SET", "0")))

    # ── Save UI → config ──────────────────────────────────────────────────
    def _ui_to_config(self):
        # Helper: convert display text → INI value
        def _ini_val(dict_key, display):
            return next((k for k, v in LANG[dict_key].items() if v == display), display)

        self.cfg["Graphics"]["ScreenWidth"]  = self._res_w_var.get().strip()
        self.cfg["Graphics"]["ScreenHeight"] = self._res_h_var.get().strip()
        self.cfg["Graphics"]["Outline"] = _ini_val("on_off", self._outline_var.get())
        self.cfg["Graphics"]["Shadow"]  = _ini_val("shadow_opt", self._shadow_var.get())

        self.cfg["Window"]["FullScreen"] = "1" if self._fullscreen_var.get() else "0"

        self.cfg["Mouse"]["MOUSE_ENABLE"] = "1" if self._mouse_var.get() else "0"

        # XInput dropdowns
        for ini_key, var in self._xinput_vars.items():
            val = var.get()
            if val:
                self.cfg["Input"][ini_key] = val

        self.cfg["Pad"]["AB_SET"] = _ini_val("ab_opt", self._abset_var.get())

    # ── Button handlers ───────────────────────────────────────────────────
    def _on_apply(self):
        self._ui_to_config()
        if save_config(self.cfg):
            self._status_var.set(f"  ✓ {LANG['msg_saved']}  {INI_PATH}")

    def _on_ok(self):
        self._ui_to_config()
        if save_config(self.cfg):
            self.destroy()

    def _on_cancel(self):
        self.destroy()


# ─── Entry Point ─────────────────────────────────────────────────────────────
if __name__ == "__main__":
    app = SettingsEditor()
    app.mainloop()
