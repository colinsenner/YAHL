#include "hook_example.h"
#include "YAHL/Detour.h"
#include <Windows.h>
#include <iostream>

struct Life {
    int a;
};

// Calling convention for this function
using fpIsCheating = bool(__cdecl *)();
using fpMeaningOfLife = void(__cdecl *)(Life &, int);

// Don't call the original and return false
bool hIsCheating(YAHL::Detour86 &detour) {
    return false;
}

// Change the a argument to be 42
void hMeaningOfLife(YAHL::Detour86 &detour, Life &life, int a) {
    detour.CallOriginal(life, 42);

    return;
}

//
float hSecretRandomNumber(YAHL::Detour86 &detour) {
    float secretNumber = detour.CallOriginal();

    return secretNumber;
}

void Example(void *hDll) {
    //
    // Hook the original IsCheating function which originally returns true that we're cheating
    //
    auto pIsCheating = (fpIsCheating)GetProcAddress(GetModuleHandle(NULL), "IsCheating");

    YAHL::Detour86 detour1(pIsCheating, &hIsCheating, 7);

    if (!detour1.Enable()) {
        std::cout << "Problem enabling detour1!\n";
    }

    //
    // Hook the MeaningOfLife function and re-call it with a different argument
    //
    auto pMeaningOfLife = (fpMeaningOfLife)GetProcAddress(GetModuleHandle(NULL), "MeaningOfLife");

    YAHL::Detour86 detour2(pMeaningOfLife, &hMeaningOfLife, 9);

    if (!detour2.Enable()) {
        std::cout << "Problem enabling detour2!\n";
    }

    while (true) {
        Sleep(50);
    }

    return;
}
