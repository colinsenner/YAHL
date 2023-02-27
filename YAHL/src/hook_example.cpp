#include "hook_example.h"
#include "YAHL/Detour.h"
#include <Windows.h>
#include <iostream>

struct Life {
    int a;
};

// Function pointers for the functions we're hooking
// We provide these so the compiler knows what value is
// returned from the original function
using fpIsCheating = bool(*)();
using fpMeaningOfLife = void(*)(Life &, int);
using fpSecretNumber = float (*)();
using fpCallConvention = int (*)(int, int);

// Don't call the original and return false
bool hIsCheating(YAHL::Detour &detour) {
    return false;
}

// Change argument to the original to be the true meaning of life, the universe, and everything
void hMeaningOfLife(YAHL::Detour &detour, Life &life, int a) {
    detour.CallOriginal<fpMeaningOfLife>(life, 42);
    return;
}

// Steal the secret number from the client and return a modified version of it
float hSecretNumber(YAHL::Detour &detour) {
    auto secretNumber = detour.CallOriginal<fpSecretNumber>();

    std::cout << "Obtained the secret number: " << secretNumber << " I'm going to add 1 to it and return it instead..."
              << std::endl;

    return secretNumber + 1;
}

// This case is a bit special, our original function is stdcall
int hCallConvention(YAHL::Detour &detour, void* returnAddress, int a, int b) {
    auto sum = detour.CallOriginal<fpCallConvention>(a, b);

    std::cout << "I'm a hooked stdcall function. I called the original and got back " << sum << std::endl;
    return sum + 1;
}

void Example(void *hDll) {
    //
    // Hook the original IsCheating function which originally returns true that we're cheating
    //
    auto pIsCheating = (fpIsCheating)GetProcAddress(GetModuleHandle(NULL), "IsCheating");

    YAHL::Detour detour1(pIsCheating, &hIsCheating, 7);

    if (!detour1.Enable()) {
        std::cout << "Problem enabling detour1!\n";
    }

    //
    // Hook the MeaningOfLife function and re-call it with a different argument
    //
    auto pMeaningOfLife = (fpMeaningOfLife)GetProcAddress(GetModuleHandle(NULL), "MeaningOfLife");

    YAHL::Detour detour2(pMeaningOfLife, &hMeaningOfLife, 9);

    if (!detour2.Enable()) {
        std::cout << "Problem enabling detour2!\n";
    }

    //
    // Hook the SecretNumber function
    //
    auto pSecretNumber = (fpMeaningOfLife)GetProcAddress(GetModuleHandle(NULL), "SecretNumber");

    YAHL::Detour detour3(pSecretNumber, &hSecretNumber, 10);

    if (!detour3.Enable()) {
        std::cout << "Problem enabling detour3!\n";
    }

    //
    // Hook the CallConvention function
    //
    auto pCallConvention = (fpCallConvention)GetProcAddress(GetModuleHandle(NULL), "_CallConvention@8");

    YAHL::Detour detour4(pCallConvention, &hCallConvention, 9);

    BREAK_ON_DEBUGGER;
    if (!detour4.Enable()) {
        std::cout << "Problem enabling detour4!\n";
    }

    while (true) {
        Sleep(50);
    }

    return;
}
