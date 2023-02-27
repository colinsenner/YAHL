#include "hook_example.h"
#include "YAHL/Detour.h"
#include <Windows.h>
#include <iostream>

struct MockLife {
    int a;
};

// Calling convention for this function
using fpIsCheating = bool(__cdecl *)();
using fpMeaningOfLife = void(__cdecl *)(MockLife &, int);
using fpSecretNumber = float (*)();

// Don't call the original and return false
bool hIsCheating(YAHL::Detour86 &detour) {
    return false;
}

// Change the a argument to be 42
void hMeaningOfLife(YAHL::Detour86 &detour, MockLife &life, int a) {
    detour.CallOriginal<fpMeaningOfLife>(life, 42);

    return;
}

// Get the secret number from the client and return a modified version of it
float hSecretNumber(YAHL::Detour86 &detour) {
    auto secretNumber = detour.CallOriginal<fpSecretNumber>();

    std::cout << "Obtained the secret number: " << secretNumber << " I'm going to add 1 to it and return it instead..." << std::endl;

    return secretNumber + 1;
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

    //
    // Hook the SecretNumber function
    //
    auto pSecretNumber = (fpMeaningOfLife)GetProcAddress(GetModuleHandle(NULL), "SecretNumber");

    YAHL::Detour86 detour3(pSecretNumber, &hSecretNumber, 10);

    if (!detour3.Enable()) {
        std::cout << "Problem enabling detour3!\n";
    }

    while (true) {
        Sleep(50);
    }

    return;
}
