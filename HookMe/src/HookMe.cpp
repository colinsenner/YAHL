#include <Windows.h>
#include <iostream>

#define DLLEXPORT extern "C" __declspec(dllexport, noinline)

struct Life {
    int meaning = 42;
};

// Make this function return false instead
DLLEXPORT bool IsCheating() {
    std::cout << "Checking if the player is cheating...\n";
    return true;
}

// Ensure this gets called, but change the a argument to be 42
DLLEXPORT void MeaningOfLife(Life &life, int a) {
    std::cout << "[MeaningOfLife] Updating the meaning of life to " << a << std::endl;
}

int main(int argc, char *argv[]) {
    Life life{};

    while (true) {
        if (IsCheating()) {
            std::cout << "[IsCheating] Player is cheating!" << std::endl;
        } else {
            std::cout << "[IsCheating] Player is NOT cheating!" << std::endl;
        }

        MeaningOfLife(life, 1);
        Sleep(1000);
    }

    return 0;
}