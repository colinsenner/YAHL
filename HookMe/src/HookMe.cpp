#include <Windows.h>
#include <cstdlib>
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

// Generate an unknowable secret number
DLLEXPORT float SecretNumber() {
    float secretNumber = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    
    return secretNumber;
}

int main(int argc, char *argv[]) {
    //srand((unsigned int)time(NULL));
    srand(1);

    Life life{};

    while (true) {
        if (IsCheating()) {
            std::cout << "[IsCheating   ] Player is cheating!" << std::endl;
        } else {
            std::cout << "[IsCheating   ] Player is NOT cheating!" << std::endl;
        }

        MeaningOfLife(life, 1);

        auto secret = SecretNumber();
        std::cout << "[SecretNumber ] Shhhhhhh....don't tell anyone the secret was " << secret << std::endl;

        Sleep(3000);
    }

    return 0;
}