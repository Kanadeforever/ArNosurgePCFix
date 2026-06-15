// CPUFix.asi - CPU 100% fix via worker thread throttle + hotkey toggle
#include <Windows.h>
#include "dxgi_hooks.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        ApplyDxgiHooks(hModule);
        break;
    case DLL_PROCESS_DETACH:
        RemoveDxgiHooks();
        break;
    }
    return TRUE;
}
