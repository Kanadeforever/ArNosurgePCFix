#pragma once
#include <Windows.h>

// ============================================================
// LocaleFix — 强制 ANSI API 使用 CP932 (Shift-JIS)
// 使任何语言系统都能正确显示日文
// ============================================================

// 字符串转换辅助(内部使用)
LPWSTR AnsiToUnicode(LPCSTR ansi, UINT codePage = 932);
LPSTR  UnicodeToAnsi(LPCWSTR wide, UINT codePage = 932);

// 初始化和卸载
BOOL ApplyHooks();   // MinHook 初始化 + 创建所有 Hook
void RemoveHooks();  // 解除所有 Hook

// ——— Tier 1: Code page core (kernel32.dll) ———
extern UINT   (WINAPI *OriginalGetACP)(void);
extern UINT   (WINAPI *OriginalGetOEMCP)(void);
extern BOOL   (WINAPI *OriginalGetCPInfo)(UINT, LPCPINFO);
extern int    (WINAPI *OriginalMultiByteToWideChar)(UINT, DWORD, LPCSTR, int, LPWSTR, int);
extern int    (WINAPI *OriginalWideCharToMultiByte)(UINT, DWORD, LPCWSTR, int, LPSTR, int, LPCSTR, LPBOOL);

// ——— Tier 2: DBCS traversal (user32.dll) ———
extern LPSTR  (WINAPI *OriginalCharPrevExA)(WORD, LPCSTR, LPCSTR, DWORD);
extern LPSTR  (WINAPI *OriginalCharNextExA)(WORD, LPCSTR, DWORD);
extern BOOL   (WINAPI *OriginalIsDBCSLeadByteEx)(UINT, BYTE);

// ——— Tier 3: Window/Text display (user32.dll) ———
extern int    (WINAPI *OriginalMessageBoxA)(HWND, LPCSTR, LPCSTR, UINT);
extern BOOL   (WINAPI *OriginalSetWindowTextA)(HWND, LPCSTR);
extern int    (WINAPI *OriginalGetWindowTextA)(HWND, LPSTR, int);
extern HWND   (WINAPI *OriginalCreateWindowExA)(DWORD, LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
extern LRESULT(WINAPI *OriginalSendMessageA)(HWND, UINT, WPARAM, LPARAM);
extern int    (WINAPI *OriginalDrawTextExA)(HDC, LPSTR, int, LPRECT, UINT, LPDRAWTEXTPARAMS);
extern HWND   (WINAPI *OriginalCreateDialogIndirectParamA)(HINSTANCE, LPCDLGTEMPLATEA, HWND, DLGPROC, LPARAM);

// ——— Tier 4: Font (gdi32.dll) ———
extern HFONT  (WINAPI *OriginalCreateFontA)(int, int, int, int, int, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, LPCSTR);
extern HFONT  (WINAPI *OriginalCreateFontIndirectA)(const LOGFONTA *);
extern HFONT  (WINAPI *OriginalCreateFontIndirectExA)(const ENUMLOGFONTEXDVA *);

// ——— Tier 5: File I/O (kernel32.dll) ———
extern HANDLE (WINAPI *OriginalCreateFileA)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
extern BOOL   (WINAPI *OriginalCreateDirectoryA)(LPCSTR, LPSECURITY_ATTRIBUTES);
