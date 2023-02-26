#include "hook_example.h"
#include "YAHL/Detour.h"
#include <Windows.h>
#include <iostream>

// Ensure this function doesn't get called
bool hFunc1(void* detour) {
    // bool isCheating = false;
    // std::cout << "[Func1] Player is NOT cheating!" << std::endl;

    printf("detour: %p\n", detour);

    return false;
}

void Example(void *hDll) {
    void *pFunc1 = GetProcAddress(GetModuleHandle(NULL), "Func1");

    typedef bool(__cdecl * fp_Func1)();

    YAHL::Detour detour(pFunc1, &hFunc1, 6);

    std::cout << "Detour object: " << &detour << std::endl;

    if (!detour.Enable()) {
        std::cout << "Problem enabling detour!\n";
    }

    while (true) {
        Sleep(50);
    }

    return;
}
