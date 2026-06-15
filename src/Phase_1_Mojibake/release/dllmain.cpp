// LocaleFix.asi — x64 ASI 插件入口
// 被 Ultimate ASI Loader (dinput8.dll) 自动加载
#include <Windows.h>
#include "../LocaleFixCommon/hooks.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        ApplyHooks();
        break;
    case DLL_PROCESS_DETACH:
        RemoveHooks();
        break;
    }
    return TRUE;
}
