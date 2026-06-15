#include "dxgi_hooks.h"
#include "../MinHook/include/MinHook.h"
#include <cstdio>
#include <cstring>
#include <cctype>
#include <mmsystem.h>
#include <Xinput.h>
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Xinput.lib")

static FILE* g_log = NULL;
#define LOG(fmt, ...) do { \
    if (g_logEnabled && g_log) { fprintf(g_log, "[CPUFix] " fmt "\n", ##__VA_ARGS__); fflush(g_log); } \
} while(0)

static HMODULE s_ourDll = NULL;
static DWORD s_mainTid = 0;
static inline BOOL IsMainThread() { return GetCurrentThreadId() == s_mainTid; }

// ====== OpenMP thread tracking ======
// Only throttle threads that have been observed entering _vcomp_barrier
// This excludes audio/Steam/input threads → fixes 2D stutter
#define MAX_OMP_THREADS 64
static DWORD s_ompThreads[MAX_OMP_THREADS] = {};
static volatile LONG s_ompCount = 0;
static SRWLOCK s_ompLock = SRWLOCK_INIT;

static BOOL IsOmpThread(DWORD tid) {
    AcquireSRWLockShared(&s_ompLock);
    for (int i = 0; i < s_ompCount; i++) {
        if (s_ompThreads[i] == tid) {
            ReleaseSRWLockShared(&s_ompLock);
            return TRUE;
        }
    }
    ReleaseSRWLockShared(&s_ompLock);
    return FALSE;
}

static void MarkOmpThread(DWORD tid) {
    AcquireSRWLockExclusive(&s_ompLock);
    for (int i = 0; i < s_ompCount; i++) {
        if (s_ompThreads[i] == tid) {
            ReleaseSRWLockExclusive(&s_ompLock);
            return;
        }
    }
    if (s_ompCount < MAX_OMP_THREADS) {
        s_ompThreads[s_ompCount++] = tid;
    }
    ReleaseSRWLockExclusive(&s_ompLock);
}

// Hook _vcomp_barrier — marks calling thread as OpenMP worker
static VOID (__cdecl *OrigVcompBarrier)(void) = NULL;
static VOID __cdecl HookVcompBarrier(void) {
    MarkOmpThread(GetCurrentThreadId());
    if (OrigVcompBarrier) OrigVcompBarrier();
}

// ====== Path helper: resolve file next to our ASI ======
static wchar_t s_asiDir[MAX_PATH] = {};
static wchar_t s_iniPath[MAX_PATH] = {};
static wchar_t s_logPath[MAX_PATH] = {};

static void InitPaths(HMODULE hModule) {
    GetModuleFileNameW(hModule, s_asiDir, MAX_PATH);
    wchar_t* slash = wcsrchr(s_asiDir, L'\\');
    if (slash) *(slash + 1) = L'\0';
    wcscpy_s(s_iniPath, s_asiDir);
    wcscat_s(s_iniPath, L"CPUFix.ini");
    wcscpy_s(s_logPath, s_asiDir);
    wcscat_s(s_logPath, L"CPUFix.log");
}

// ====== Config ======
static int  g_throttleMs = 1;
static int  g_enableOnStart = 1;
static int  g_logEnabled = 0;
static UINT g_hotkeyMod = 0;
static UINT g_hotkeyVk  = VK_F1;
static WORD g_gamepadCombo = XINPUT_GAMEPAD_BACK | XINPUT_GAMEPAD_RIGHT_THUMB;

static void WriteDefaultIni() {
    FILE* f = NULL;
    _wfopen_s(&f, s_iniPath, L"w");
    if (f) {
        fprintf(f,
            "; ================================================================\n"
            "; CPUFix for Ar nosurge DX / Ciel nosurge DX\n"
            "; ----------------------------------------------------------------\n"
            "; [EN] Throttle delay in milliseconds (1-16).\n"
            ";      Higher values lower CPU more but may reduce framerate.\n"
            ";      Default: 1 (recommended — do not change unless stuttering).\n"
            "; [JP] スロットル遅延(ミリ秒, 1-16)。\n"
            ";      値を上げるとCPU使用率は下がるがフレームレートも低下。\n"
            ";      デフォルト: 1 (推奨 — カクつく場合以外は変えないでください)。\n"
            "; [CN] 节流延迟(毫秒, 1-16)。\n"
            ";      值越大CPU占用越低但帧率可能下降。\n"
            ";      默认: 1 (推荐 — 除非卡顿否则不要改动)。\n"
            "ThrottleMs=1\n"
            "\n"
            "; [EN] Throttle ON at game start? 1 = ON, 0 = OFF.\n"
            "; [JP] ゲーム起動時にスロットルを有効にするか？ 1 = オン, 0 = オフ。\n"
            "; [CN] 启动时是否开关节流？ 1 = 开, 0 = 关。\n"
            "EnableOnStart=1\n"
            "\n"
            "; [EN] Keyboard hotkey to toggle throttle.\n"
            ";      Format: [MODIFIER+]KEY (e.g. F1, CTRL+SHIFT+F, ALT+BACK).\n"
            ";      Supported keys: F1-F12, A-Z, 0-9, BACK, TAB, RETURN, SPACE, ESCAPE.\n"
            ";      Supported modifiers: CTRL, SHIFT, ALT.\n"
            "; [JP] スロットル切り替えのキーボードホットキー。\n"
            ";      書式: [修飾キー+]キー (例: F1, CTRL+SHIFT+F, ALT+BACK)。\n"
            ";      対応キー: F1-F12, A-Z, 0-9, BACK, TAB, RETURN, SPACE, ESCAPE。\n"
            ";      対応修飾キー: CTRL, SHIFT, ALT。\n"
            "; [CN] 键盘切换节流热键。\n"
            ";      格式: [修饰键+]按键 (例: F1, CTRL+SHIFT+F, ALT+BACK)。\n"
            ";      支持按键: F1-F12, A-Z, 0-9, BACK, TAB, RETURN, SPACE, ESCAPE。\n"
            ";      支持修饰键: CTRL, SHIFT, ALT。\n"
            "Hotkey=F1\n"
            "\n"
            "; [EN] Gamepad button combo to toggle throttle (XInput).\n"
            ";      Format: BUTTON[+BUTTON] (e.g. BACK+R3, LB+RB, START+BACK).\n"
            ";      Supported: A, B, X, Y, LB, RB, BACK, START, L3, R3,\n"
            ";                 DPAD_U, DPAD_D, DPAD_L, DPAD_R.\n"
            ";      Leave empty to disable gamepad toggle.\n"
            "; [JP] スロットル切り替えのゲームパッドボタン (XInput)。\n"
            ";      書式: ボタン[+ボタン] (例: BACK+R3, LB+RB, START+BACK)。\n"
            ";      対応: A, B, X, Y, LB, RB, BACK, START, L3, R3,\n"
            ";              DPAD_U, DPAD_D, DPAD_L, DPAD_R。\n"
            ";      空欄でゲームパッド無効。\n"
            "; [CN] 手柄切换节流按钮组合 (XInput)。\n"
            ";      格式: 按钮[+按钮] (例: BACK+R3, LB+RB, START+BACK)。\n"
            ";      支持: A, B, X, Y, LB, RB, BACK, START, L3, R3,\n"
            ";              DPAD_U, DPAD_D, DPAD_L, DPAD_R。\n"
            ";      留空禁用手柄切换。\n"
            "GamepadHotkey=BACK+R3\n"
            "\n"
            "; [EN] Enable diagnostic log file (CPUFix.log)? 1 = yes, 0 = no.\n"
            "; [JP] 診断ログ(CPUFix.log)を有効にするか？ 1 = はい, 0 = いいえ。\n"
            "; [CN] 是否启用诊断日志(CPUFix.log)？ 1 = 是, 0 = 否。\n"
            "LogEnabled=0\n"
        );
        fclose(f);
    }
}

static void LoadConfig() {
    if (GetFileAttributesW(s_iniPath) == INVALID_FILE_ATTRIBUTES)
        WriteDefaultIni();
    g_throttleMs = GetPrivateProfileIntW(L"CPUFix", L"ThrottleMs", 1, s_iniPath);
    if (g_throttleMs < 1) g_throttleMs = 1;
    if (g_throttleMs > 16) g_throttleMs = 16;
    g_enableOnStart = GetPrivateProfileIntW(L"CPUFix", L"EnableOnStart", 1, s_iniPath);
    g_logEnabled    = GetPrivateProfileIntW(L"CPUFix", L"LogEnabled", 0, s_iniPath);

    wchar_t whk[128], wgp[128];
    GetPrivateProfileStringW(L"CPUFix", L"Hotkey", L"F1", whk, 128, s_iniPath);
    GetPrivateProfileStringW(L"CPUFix", L"GamepadHotkey", L"BACK+R3", wgp, 128, s_iniPath);
    char hk[128], gp[128];
    WideCharToMultiByte(CP_UTF8, 0, whk, -1, hk, 128, NULL, NULL);
    WideCharToMultiByte(CP_UTF8, 0, wgp, -1, gp, 128, NULL, NULL);

    // Parse keyboard hotkey (stripped down: just key name with optional modifiers)
    char buf[128];
    strncpy_s(buf, hk, 127);
    char* lp = strrchr(buf, '+');
    char* kn = lp ? lp + 1 : buf;
    if (lp) { *lp = 0; g_hotkeyMod = 0;
        if (strstr(buf, "CTRL"))  g_hotkeyMod |= MOD_CONTROL;
        if (strstr(buf, "SHIFT")) g_hotkeyMod |= MOD_SHIFT;
        if (strstr(buf, "ALT"))   g_hotkeyMod |= MOD_ALT;
    }
    // Parse key name
    if (!_stricmp(kn, "F1")) g_hotkeyVk = VK_F1; else if (!_stricmp(kn, "F2")) g_hotkeyVk = VK_F2;
    else if (!_stricmp(kn, "F3")) g_hotkeyVk = VK_F3; else if (!_stricmp(kn, "F4")) g_hotkeyVk = VK_F4;
    else if (!_stricmp(kn, "F5")) g_hotkeyVk = VK_F5; else if (!_stricmp(kn, "F6")) g_hotkeyVk = VK_F6;
    else if (!_stricmp(kn, "F7")) g_hotkeyVk = VK_F7; else if (!_stricmp(kn, "F8")) g_hotkeyVk = VK_F8;
    else if (!_stricmp(kn, "F9")) g_hotkeyVk = VK_F9; else if (!_stricmp(kn, "F10")) g_hotkeyVk = VK_F10;
    else if (!_stricmp(kn, "F11")) g_hotkeyVk = VK_F11; else if (!_stricmp(kn, "F12")) g_hotkeyVk = VK_F12;
    else if (!_stricmp(kn, "BACK")||!_stricmp(kn,"BACKSPACE")) g_hotkeyVk = VK_BACK;
    else if (!_stricmp(kn, "TAB")) g_hotkeyVk = VK_TAB;
    else if (!_stricmp(kn, "RETURN")||!_stricmp(kn,"ENTER")) g_hotkeyVk = VK_RETURN;
    else if (!_stricmp(kn, "SPACE")) g_hotkeyVk = VK_SPACE;
    else if (!_stricmp(kn, "ESCAPE")||!_stricmp(kn,"ESC")) g_hotkeyVk = VK_ESCAPE;
    else if (strlen(kn) == 1) { char c = (char)toupper(kn[0]); if ((c>='A'&&c<='Z')||(c>='0'&&c<='9')) g_hotkeyVk = c; }

    // Parse gamepad
    g_gamepadCombo = 0;
    if (gp[0]) {
        char* ctx = NULL; char* tok = strtok_s(gp, "+", &ctx);
        while (tok) {
            while (*tok==' ') tok++;
            if (!_stricmp(tok,"A")) g_gamepadCombo|=XINPUT_GAMEPAD_A; else if (!_stricmp(tok,"B")) g_gamepadCombo|=XINPUT_GAMEPAD_B;
            else if (!_stricmp(tok,"X")) g_gamepadCombo|=XINPUT_GAMEPAD_X; else if (!_stricmp(tok,"Y")) g_gamepadCombo|=XINPUT_GAMEPAD_Y;
            else if (!_stricmp(tok,"LB")) g_gamepadCombo|=XINPUT_GAMEPAD_LEFT_SHOULDER; else if (!_stricmp(tok,"RB")) g_gamepadCombo|=XINPUT_GAMEPAD_RIGHT_SHOULDER;
            else if (!_stricmp(tok,"BACK")) g_gamepadCombo|=XINPUT_GAMEPAD_BACK; else if (!_stricmp(tok,"START")) g_gamepadCombo|=XINPUT_GAMEPAD_START;
            else if (!_stricmp(tok,"L3")) g_gamepadCombo|=XINPUT_GAMEPAD_LEFT_THUMB; else if (!_stricmp(tok,"R3")) g_gamepadCombo|=XINPUT_GAMEPAD_RIGHT_THUMB;
            else if (!_stricmp(tok,"DPAD_U")||!_stricmp(tok,"DPAD_UP")) g_gamepadCombo|=XINPUT_GAMEPAD_DPAD_UP;
            else if (!_stricmp(tok,"DPAD_D")||!_stricmp(tok,"DPAD_DOWN")) g_gamepadCombo|=XINPUT_GAMEPAD_DPAD_DOWN;
            else if (!_stricmp(tok,"DPAD_L")||!_stricmp(tok,"DPAD_LEFT")) g_gamepadCombo|=XINPUT_GAMEPAD_DPAD_LEFT;
            else if (!_stricmp(tok,"DPAD_R")||!_stricmp(tok,"DPAD_RIGHT")) g_gamepadCombo|=XINPUT_GAMEPAD_DPAD_RIGHT;
            tok = strtok_s(NULL, "+", &ctx);
        }
    }
}

// ====== Throttle state & toggle ======
static volatile LONG s_throttleOn = 1;

static void DoToggle() {
    LONG prev = InterlockedExchange(&s_throttleOn, !s_throttleOn);
    LOG("TOGGLE: %s (omp threads=%ld)", s_throttleOn ? "ON" : "OFF", s_ompCount);
}

// ====== Hotkey ======
static LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_HOTKEY) DoToggle();
    return DefWindowProcW(h, m, w, l);
}

// ====== Gamepad poll ======
static volatile LONG s_padRunning = 1;
static DWORD WINAPI GamepadThread(LPVOID) {
    WORD prev = 0;
    while (s_padRunning) {
        Sleep(100);
        if (!g_gamepadCombo) continue;
        XINPUT_STATE st;
        if (XInputGetState(0, &st) == ERROR_SUCCESS) {
            WORD now = st.Gamepad.wButtons;
            WORD jp = now & ~prev;
            if ((now & g_gamepadCombo) == g_gamepadCombo && (jp & g_gamepadCombo))
                DoToggle();
            prev = now;
        }
    }
    return 0;
}

// ====== Message pump thread ======
static volatile LONG s_msgRunning = 1;
static DWORD s_msgTid = 0;
static DWORD WINAPI MsgThread(LPVOID) {
    WNDCLASSW wc = {}; wc.lpfnWndProc = WndProc; wc.hInstance = s_ourDll; wc.lpszClassName = L"CPUFixWnd";
    RegisterClassW(&wc);
    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"", 0,0,0,0,0, HWND_MESSAGE, NULL, s_ourDll, NULL);
    if (hwnd && g_hotkeyVk) RegisterHotKey(hwnd, 1, g_hotkeyMod, g_hotkeyVk);

    HANDLE hPad = CreateThread(NULL, 0, GamepadThread, NULL, 0, NULL);
    if (hPad) CloseHandle(hPad);

    MSG msg;
    while (s_msgRunning && GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg); DispatchMessageW(&msg);
    }
    s_padRunning = 0;
    if (hwnd) DestroyWindow(hwnd);
    return 0;
}

// ====== Hooks — only throttle OpenMP worker threads ======
static VOID (WINAPI *OrigSleep)(DWORD) = Sleep;
static VOID WINAPI HookSleep(DWORD ms) {
    if (ms == 0 && !IsMainThread() && IsOmpThread(GetCurrentThreadId()) && s_throttleOn)
        { OrigSleep(g_throttleMs); return; }
    OrigSleep(ms);
}

static BOOL (WINAPI *OrigSwitchToThread)(void) = SwitchToThread;
static BOOL WINAPI HookSwitchToThread(void) {
    if (!IsMainThread() && IsOmpThread(GetCurrentThreadId()) && s_throttleOn)
        { Sleep(g_throttleMs); return TRUE; }
    return OrigSwitchToThread();
}

static DWORD (WINAPI *OrigWaitForSingleObject)(HANDLE, DWORD) = WaitForSingleObject;
static DWORD WINAPI HookWaitForSingleObject(HANDLE h, DWORD ms) {
    if (ms == 0 && !IsMainThread() && IsOmpThread(GetCurrentThreadId()) && s_throttleOn)
        return OrigWaitForSingleObject(h, g_throttleMs);
    return OrigWaitForSingleObject(h, ms);
}

// ====== Init ======
BOOL ApplyDxgiHooks(HMODULE hModule) {
    s_ourDll = hModule;
    InitPaths(hModule);
    s_mainTid = GetCurrentThreadId();
    LoadConfig();
    if (g_logEnabled) _wfopen_s(&g_log, s_logPath, L"w");
    s_throttleOn = g_enableOnStart;
    LOG("CPUFix v16 — omp-only. Ms=%d start=%s", g_throttleMs, s_throttleOn ? "ON":"OFF");
    timeBeginPeriod(1);

    if (MH_Initialize() != MH_OK) { if (g_log) fclose(g_log); g_log = NULL; return FALSE; }
    MH_CreateHookApi(L"kernel32.dll", "Sleep",               HookSleep,               (LPVOID*)&OrigSleep);
    MH_CreateHookApi(L"kernel32.dll", "SwitchToThread",      HookSwitchToThread,      (LPVOID*)&OrigSwitchToThread);
    MH_CreateHookApi(L"kernel32.dll", "WaitForSingleObject", HookWaitForSingleObject, (LPVOID*)&OrigWaitForSingleObject);

    // Hook vcomp140 barrier to detect OpenMP threads
    HMODULE hv = LoadLibraryW(L"vcomp140.dll");
    if (hv) {
        OrigVcompBarrier = (VOID(__cdecl*)(void))GetProcAddress(hv, "_vcomp_barrier");
        if (OrigVcompBarrier) {
            MH_CreateHook(OrigVcompBarrier, HookVcompBarrier, (LPVOID*)&OrigVcompBarrier);
            LOG("_vcomp_barrier hooked");
        }
    }

    MH_EnableHook(MH_ALL_HOOKS);
    HANDLE h = CreateThread(NULL, 0, MsgThread, NULL, 0, &s_msgTid);
    if (h) CloseHandle(h);
    return TRUE;
}

void RemoveDxgiHooks() {
    s_msgRunning = 0;
    if (s_msgTid) PostThreadMessageW(s_msgTid, WM_QUIT, 0, 0);
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();
    timeEndPeriod(1);
    if (g_log) { LOG("Unloaded (omp=%ld)", s_ompCount); fclose(g_log); g_log = NULL; }
}
