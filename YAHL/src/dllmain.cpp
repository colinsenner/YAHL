// dllmain.cpp : Defines the entry point for the DLL application.
#include "hook_example.h"
#include <thread>
#include <Windows.h>

// Guards against multiple attach calls
static bool bInitializeCalled = false;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);

        // Guard against multiple calls.
        if (bInitializeCalled) {
            FreeLibrary(hModule);
            return FALSE;
        }

        bInitializeCalled = true;

        _beginthread(&Example, 0, (void*)hModule);
        break;
    }
    return TRUE;
}
