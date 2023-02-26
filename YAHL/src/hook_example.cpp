#include "hook_example.h"
#include "YAHL/Detour.h"
#include <Windows.h>
#include <iostream>

// Calling convention for this function
using fpFunc1 = bool(__cdecl *)();

// Ensure this function doesn't get called
bool hFunc1(YAHL::Detour86<fpFunc1> *detour) {
    bool isCheating = false;
    std::cout << "[Func1] Player is NOT cheating!" << std::endl;
    
    //detour->CallOriginal();

    return isCheating;
}

void Example(void *hDll) {
    void *pFunc1 = GetProcAddress(GetModuleHandle(NULL), "Func1");

    YAHL::Detour86<fpFunc1> detour(pFunc1, &hFunc1, 6);

    std::cout << "Detour object: " << &detour << std::endl;

    if (!detour.Enable()) {
        std::cout << "Problem enabling detour!\n";
    }

    while (true) {
        Sleep(50);
    }

    return;
}
