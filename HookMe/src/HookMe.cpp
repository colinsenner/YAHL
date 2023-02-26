#include <Windows.h>
#include <iostream>

#define DLLEXPORT extern "C" __declspec(dllexport, noinline)

struct Life {
    int meaning = 42;
};

// Ensure this function doesn't get called
DLLEXPORT bool Func1() {
    bool isCheating = true;
    std::cout << "[Func1] Player is cheating!" << std::endl;
    return isCheating;
}

// Ensure this gets called, but we get first chance at it
DLLEXPORT void Func2(Life &life, int a) {
    std::cout << "[Func2] Updating the meaning of life to " << a << std::endl;
}

int main(int argc, char *argv[]) {
    Life life{};

    while (true) {
        Func1();
        Func2(life, 42);
        Sleep(1000);
    }

    return 0;
}