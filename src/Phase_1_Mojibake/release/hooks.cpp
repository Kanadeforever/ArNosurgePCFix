// LocaleFix — caller-filtered code page hooks
// Only redirect ANSI→CP932 for calls originating from the game EXE.
// System DLL calls pass through unchanged.
#include "hooks.h"
#include "../MinHook/include/MinHook.h"
#include <intrin.h>

// Tier 1: Code page core
UINT  (WINAPI *OriginalGetACP)(void) = NULL;
UINT  (WINAPI *OriginalGetOEMCP)(void) = NULL;
BOOL  (WINAPI *OriginalGetCPInfo)(UINT, LPCPINFO) = NULL;
int   (WINAPI *OriginalMultiByteToWideChar)(UINT, DWORD, LPCSTR, int, LPWSTR, int) = NULL;
int   (WINAPI *OriginalWideCharToMultiByte)(UINT, DWORD, LPCWSTR, int, LPSTR, int, LPCSTR, LPBOOL) = NULL;

// Stubs for unused tiers
LPSTR  (WINAPI *OriginalCharPrevExA)(WORD, LPCSTR, LPCSTR, DWORD) = NULL;
LPSTR  (WINAPI *OriginalCharNextExA)(WORD, LPCSTR, DWORD) = NULL;
BOOL   (WINAPI *OriginalIsDBCSLeadByteEx)(UINT, BYTE) = NULL;
int    (WINAPI *OriginalMessageBoxA)(HWND, LPCSTR, LPCSTR, UINT) = NULL;
BOOL   (WINAPI *OriginalSetWindowTextA)(HWND, LPCSTR) = NULL;
int    (WINAPI *OriginalGetWindowTextA)(HWND, LPSTR, int) = NULL;
HWND   (WINAPI *OriginalCreateWindowExA)(DWORD, LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID) = NULL;
LRESULT(WINAPI *OriginalSendMessageA)(HWND, UINT, WPARAM, LPARAM) = NULL;
int    (WINAPI *OriginalDrawTextExA)(HDC, LPSTR, int, LPRECT, UINT, LPDRAWTEXTPARAMS) = NULL;
HWND   (WINAPI *OriginalCreateDialogIndirectParamA)(HINSTANCE, LPCDLGTEMPLATEA, HWND, DLGPROC, LPARAM) = NULL;
HFONT  (WINAPI *OriginalCreateFontA)(int,int,int,int,int,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,LPCSTR) = NULL;
HFONT  (WINAPI *OriginalCreateFontIndirectA)(const LOGFONTA *) = NULL;
HFONT  (WINAPI *OriginalCreateFontIndirectExA)(const ENUMLOGFONTEXDVA *) = NULL;
HANDLE (WINAPI *OriginalCreateFileA)(LPCSTR,DWORD,DWORD,LPSECURITY_ATTRIBUTES,DWORD,DWORD,HANDLE) = NULL;
BOOL   (WINAPI *OriginalCreateDirectoryA)(LPCSTR, LPSECURITY_ATTRIBUTES) = NULL;

// Helpers
static HANDLE s_heap = NULL;
static void InitHeap(void) { if (!s_heap) s_heap = GetProcessHeap(); }

LPWSTR AnsiToUnicode(LPCSTR ansi, UINT cp) {
    if (!ansi) return NULL;
    InitHeap();
    int len = lstrlenA(ansi);
    int wsz = MultiByteToWideChar(cp, 0, ansi, len, NULL, 0);
    if (wsz <= 0) return NULL;
    LPWSTR w = (LPWSTR)HeapAlloc(s_heap, HEAP_ZERO_MEMORY, (wsz + 1) * sizeof(WCHAR));
    if (w) { MultiByteToWideChar(cp, 0, ansi, len, w, wsz); w[wsz] = 0; }
    return w;
}
LPSTR UnicodeToAnsi(LPCWSTR wide, UINT cp) {
    if (!wide) return NULL; InitHeap();
    int len = lstrlenW(wide);
    int asz = WideCharToMultiByte(cp, 0, wide, len, NULL, 0, NULL, NULL);
    if (asz <= 0) return NULL;
    LPSTR a = (LPSTR)HeapAlloc(s_heap, HEAP_ZERO_MEMORY, asz + 1);
    if (a) { WideCharToMultiByte(cp, 0, wide, len, a, asz, NULL, NULL); a[asz] = 0; }
    return a;
}

// Check if caller is game EXE
static BOOL IsGameCaller(void) {
    HMODULE caller = NULL;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)_ReturnAddress(), &caller);
    return caller == GetModuleHandleW(NULL);
}

// ====== Tier 1 Hooks (caller-filtered) ======
static UINT WINAPI HookGetACP(void) {
    if (IsGameCaller()) return 932;
    return OriginalGetACP();
}
static UINT WINAPI HookGetOEMCP(void) {
    if (IsGameCaller()) return 932;
    return OriginalGetOEMCP();
}
static BOOL WINAPI HookGetCPInfo(UINT cp, LPCPINFO info) {
    if (IsGameCaller()) return OriginalGetCPInfo(932, info);
    return OriginalGetCPInfo(cp, info);
}
static int WINAPI HookMultiByteToWideChar(UINT cp, DWORD fl, LPCSTR mb, int mbsz, LPWSTR wc, int wcsz) {
    if (IsGameCaller() && cp < CP_UTF7) cp = 932;
    return OriginalMultiByteToWideChar(cp, fl, mb, mbsz, wc, wcsz);
}
static int WINAPI HookWideCharToMultiByte(UINT cp, DWORD fl, LPCWSTR wc, int wcsz, LPSTR mb, int mbsz, LPCSTR def, LPBOOL used) {
    if (IsGameCaller() && cp < CP_UTF7) cp = 932;
    return OriginalWideCharToMultiByte(cp, fl, wc, wcsz, mb, mbsz, def, used);
}

// ====== Tier 3: Window text hooks ======
static BOOL WINAPI HookSetWindowTextA(HWND hWnd, LPCSTR lpString) {
    if (IsGameCaller() && lpString) {
        LPWSTR w = AnsiToUnicode(lpString, 932);
        BOOL r = SetWindowTextW(hWnd, w ? w : L"");
        HeapFree(s_heap, 0, w);
        return r;
    }
    return OriginalSetWindowTextA(hWnd, lpString);
}

static HWND WINAPI HookCreateWindowExA(DWORD ex, LPCSTR cls, LPCSTR name, DWORD st,
    int x, int y, int w, int h, HWND p, HMENU m, HINSTANCE i, LPVOID lp)
{
    if (!IsGameCaller())
        return OriginalCreateWindowExA(ex, cls, name, st, x, y, w, h, p, m, i, lp);

    // lpClassName may be IS_INTRESOURCE (atom) — must not convert
    LPWSTR wCls = NULL, wName = NULL;
    if (cls && !IS_INTRESOURCE(cls)) wCls  = AnsiToUnicode(cls, 932);
    if (name)                       wName = AnsiToUnicode(name, 932);
    HWND r = CreateWindowExW(ex, wCls ? wCls : (LPCWSTR)(ULONG_PTR)cls, wName, st, x, y, w, h, p, m, i, lp);
    HeapFree(s_heap, 0, wCls);
    HeapFree(s_heap, 0, wName);
    return r;
}

static int WINAPI HookMessageBoxA(HWND h, LPCSTR text, LPCSTR cap, UINT type) {
    if (!IsGameCaller())
        return OriginalMessageBoxA(h, text, cap, type);
    LPWSTR wt = AnsiToUnicode(text, 932), wc = AnsiToUnicode(cap, 932);
    int r = MessageBoxW(h, wt ? wt : L"", wc ? wc : L"", type);
    HeapFree(s_heap, 0, wt); HeapFree(s_heap, 0, wc);
    return r;
}

static LRESULT WINAPI HookSendMessageA(HWND h, UINT msg, WPARAM wp, LPARAM lp) {
    if (!IsGameCaller())
        return OriginalSendMessageA(h, msg, wp, lp);

    switch (msg) {
    // String SET: lParam is ANSI string → convert to Unicode, send to W version
    case WM_SETTEXT:
    case WM_SETTINGCHANGE:
    case EM_REPLACESEL:
    case CB_ADDSTRING: case CB_INSERTSTRING:
    case CB_FINDSTRING: case CB_SELECTSTRING: case CB_FINDSTRINGEXACT:
    case LB_ADDSTRING: case LB_INSERTSTRING:
    case LB_FINDSTRING: case LB_SELECTSTRING: case LB_FINDSTRINGEXACT:
    case CB_DIR: case LB_DIR: case LB_ADDFILE:
    case WM_DEVMODECHANGE:
    {
        LPWSTR w = AnsiToUnicode((LPCSTR)lp, 932);
        LRESULT r = SendMessageW(h, msg, wp, (LPARAM)w);
        HeapFree(s_heap, 0, w);
        return r;
    }
    // String GET: get Unicode first, then convert back to CP932 for caller
    case WM_GETTEXT:
    case CB_GETLBTEXT: case LB_GETTEXT:
    {
        int n = (int)wp;
        LPWSTR w = (LPWSTR)HeapAlloc(s_heap, HEAP_ZERO_MEMORY, (n + 1) * sizeof(WCHAR));
        if (!w) return 0;
        LRESULT len = SendMessageW(h, msg == WM_GETTEXT ? WM_GETTEXT : (msg + 0x2000), wp, (LPARAM)w);
        if (len > 0)
            len = WideCharToMultiByte(932, 0, w, (int)len, (LPSTR)lp, n, NULL, NULL);
        HeapFree(s_heap, 0, w);
        return len;
    }
    default:
        return OriginalSendMessageA(h, msg, wp, lp);
    }
}

// ====== Init ======
BOOL ApplyHooks(void) {
    InitHeap();
    if (MH_Initialize() != MH_OK) return FALSE;

    MH_CreateHookApi(L"kernel32.dll", "GetACP",              HookGetACP,              (LPVOID*)&OriginalGetACP);
    MH_CreateHookApi(L"kernel32.dll", "GetOEMCP",            HookGetOEMCP,            (LPVOID*)&OriginalGetOEMCP);
    MH_CreateHookApi(L"kernel32.dll", "GetCPInfo",           HookGetCPInfo,           (LPVOID*)&OriginalGetCPInfo);
    MH_CreateHookApi(L"kernel32.dll", "MultiByteToWideChar", HookMultiByteToWideChar, (LPVOID*)&OriginalMultiByteToWideChar);
    MH_CreateHookApi(L"kernel32.dll", "WideCharToMultiByte", HookWideCharToMultiByte, (LPVOID*)&OriginalWideCharToMultiByte);

    // Tier 3: Window text (user32.dll)
    MH_CreateHookApi(L"user32.dll", "SetWindowTextA",  HookSetWindowTextA,  (LPVOID*)&OriginalSetWindowTextA);
    MH_CreateHookApi(L"user32.dll", "CreateWindowExA", HookCreateWindowExA, (LPVOID*)&OriginalCreateWindowExA);
    MH_CreateHookApi(L"user32.dll", "MessageBoxA",     HookMessageBoxA,     (LPVOID*)&OriginalMessageBoxA);
    MH_CreateHookApi(L"user32.dll", "SendMessageA",    HookSendMessageA,    (LPVOID*)&OriginalSendMessageA);

    MH_EnableHook(MH_ALL_HOOKS);
    return TRUE;
}

void RemoveHooks(void) { MH_DisableHook(MH_ALL_HOOKS); MH_Uninitialize(); }
