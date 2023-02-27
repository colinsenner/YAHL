#include "hook_example.h"
#include "YAHL/Detour.h"
#include <Windows.h>
#include <iostream>

struct Life {
    int a;
};

// Calling convention for this function
using fpIsCheating = bool(__cdecl *)();
using fpMeaningOfLife = void(__cdecl *)(Life&, int);

// Don't call the original and return false
bool hIsCheating(YAHL::Detour86<fpIsCheating> *detour) {
    return false;
}

// Change the a argument to be 42
void hMeaningOfLife(YAHL::Detour86<fpMeaningOfLife> *detour, Life &life, int a) {
    return detour->CallOriginal(life, 42);
}

void Example(void *hDll) {
    fpIsCheating pIsCheating = (fpIsCheating)GetProcAddress(GetModuleHandle(NULL), "IsCheating");
    YAHL::Detour86<fpIsCheating> detour1(pIsCheating, &hIsCheating, 7);
    std::cout << "IsCheatingDetour: " << &detour1 << std::endl;

    fpMeaningOfLife pMeaningOfLife = (fpMeaningOfLife)GetProcAddress(GetModuleHandle(NULL), "MeaningOfLife");
    YAHL::Detour86<fpMeaningOfLife> detour2(pMeaningOfLife, &hMeaningOfLife, 9);
    std::cout << "MeaningOfLifeDetour: " << &detour2 << std::endl;

    if (!detour1.Enable()) {
        std::cout << "Problem enabling detour1!\n";
    }

    if (!detour2.Enable()) {
        std::cout << "Problem enabling detour2!\n";
    }

    while (true) {
        Sleep(50);
    }

    return;
}
