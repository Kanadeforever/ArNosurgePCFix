// CPUFix Lite — always-on, no config, no hotkey, minimal
#include <Windows.h>
#include <cstdio>
#include <mmsystem.h>
#include "../MinHook/include/MinHook.h"
#pragma comment(lib, "winmm.lib")

static FILE* g_log = NULL;
#define LOG(fmt, ...) do { \
    if (g_log) { fprintf(g_log, "[CPUFix] " fmt "\n", ##__VA_ARGS__); fflush(g_log); } \
} while(0)

static DWORD s_mainTid = 0;
static inline BOOL IsMainThread(void) { return GetCurrentThreadId() == s_mainTid; }

// OpenMP thread tracking
#define MAX_OMP 64
static DWORD  s_omp[MAX_OMP] = {};
static LONG   s_ompCnt = 0;
static SRWLOCK s_ompLock = SRWLOCK_INIT;

static BOOL IsOmpThread(DWORD tid) {
    AcquireSRWLockShared(&s_ompLock);
    for (int i = 0; i < s_ompCnt; i++) { if (s_omp[i] == tid) { ReleaseSRWLockShared(&s_ompLock); return TRUE; } }
    ReleaseSRWLockShared(&s_ompLock);
    return FALSE;
}
static void MarkOmp(DWORD tid) {
    AcquireSRWLockExclusive(&s_ompLock);
    for (int i = 0; i < s_ompCnt; i++) { if (s_omp[i] == tid) { ReleaseSRWLockExclusive(&s_ompLock); return; } }
    if (s_ompCnt < MAX_OMP) s_omp[s_ompCnt++] = tid;
    ReleaseSRWLockExclusive(&s_ompLock);
}

// _vcomp_barrier hook — marks OpenMP threads
static VOID (__cdecl *OrigVcompBarrier)(void) = NULL;
static VOID __cdecl HookVcompBarrier(void) {
    MarkOmp(GetCurrentThreadId());
    if (OrigVcompBarrier) OrigVcompBarrier();
}

// Worker thread throttling
static VOID (WINAPI *OrigSleep)(DWORD) = Sleep;
static VOID WINAPI HookSleep(DWORD ms) {
    if (ms == 0 && !IsMainThread() && IsOmpThread(GetCurrentThreadId())) { OrigSleep(1); return; }
    OrigSleep(ms);
}
static BOOL (WINAPI *OrigSwitchToThread)(void) = SwitchToThread;
static BOOL WINAPI HookSwitchToThread(void) {
    if (!IsMainThread() && IsOmpThread(GetCurrentThreadId())) { Sleep(1); return TRUE; }
    return OrigSwitchToThread();
}
static DWORD (WINAPI *OrigWaitForSingleObject)(HANDLE, DWORD) = WaitForSingleObject;
static DWORD WINAPI HookWaitForSingleObject(HANDLE h, DWORD ms) {
    if (ms == 0 && !IsMainThread() && IsOmpThread(GetCurrentThreadId())) return OrigWaitForSingleObject(h, 1);
    return OrigWaitForSingleObject(h, ms);
}

// Init / Cleanup
BOOL ApplyDxgiHooks(HMODULE hModule) {
    timeBeginPeriod(1);
    s_mainTid = GetCurrentThreadId();

    // Log next to ASI
    wchar_t lp[MAX_PATH];
    GetModuleFileNameW(hModule, lp, MAX_PATH);
    wchar_t* sl = wcsrchr(lp, L'\\'); if (sl) *(sl + 1) = L'\0';
    wcscat_s(lp, L"CPUFix.log");
    _wfopen_s(&g_log, lp, L"w");
    LOG("CPUFix Lite — always-on OMP throttle");

    if (MH_Initialize() != MH_OK) { if (g_log) fclose(g_log); g_log = NULL; return FALSE; }

    MH_CreateHookApi(L"kernel32.dll", "Sleep",               HookSleep,               (LPVOID*)&OrigSleep);
    MH_CreateHookApi(L"kernel32.dll", "SwitchToThread",      HookSwitchToThread,      (LPVOID*)&OrigSwitchToThread);
    MH_CreateHookApi(L"kernel32.dll", "WaitForSingleObject", HookWaitForSingleObject, (LPVOID*)&OrigWaitForSingleObject);

    HMODULE hv = LoadLibraryW(L"vcomp140.dll");
    if (hv) {
        OrigVcompBarrier = (VOID(__cdecl*)(void))GetProcAddress(hv, "_vcomp_barrier");
        if (OrigVcompBarrier) { MH_CreateHook(OrigVcompBarrier, HookVcompBarrier, (LPVOID*)&OrigVcompBarrier); LOG("_vcomp_barrier hooked"); }
    }

    MH_EnableHook(MH_ALL_HOOKS);
    LOG("Armed — %ld OMP threads tracked", s_ompCnt);
    return TRUE;
}

void RemoveDxgiHooks(void) {
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();
    timeEndPeriod(1);
    if (g_log) { LOG("Unloaded (omp=%ld)", s_ompCnt); fclose(g_log); g_log = NULL; }
}
