#include "Detour86.h"
#include <Windows.h>
#include <iostream>
#include <vector>

const uint32_t stubSize = 256;

enum class AsmIns : uint8_t {
    Int3 = 0xCC,
    Push = 0x68,
    Call = 0xE8,
    Ret = 0xC3,
    Nop = 0x90,
    Jmp = 0xE9,
    Add = 0x83,
    Esp = 0xC4
};

#define BREAK_ON_DEBUGGER                                                                                              \
    if (IsDebuggerPresent()) {                                                                                         \
        DebugBreak();                                                                                                  \
    }

bool YAHL::Detour86::Enable() {
    if (numBytesToHook_ > originalBytes_.max_size()) {
        return false;
    }

    SaveOriginalBytes();

    if (!CreateStub()) {
        return false;
    }

    if (!WriteStub()) {
        return false;
    }

    if (!WriteTrampoline()) {
        return false;
    }

    std::cout << "Hook enabled!\n";
    return true;
}

void YAHL::Detour86::CallOriginal() {
}

void YAHL::Detour86::SaveOriginalBytes() {
    memcpy(originalBytes_.data(), originalFunction_, numBytesToHook_);
}

bool YAHL::Detour86::CreateStub() {
    stub_ = (uint8_t *)VirtualAlloc(NULL, stubSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    return stub_ != NULL;
}

template <typename T> void serialize(std::vector<uint8_t> &v, const T &obj) {
    static_assert(std::is_trivially_copyable<T>::value, "Can only serialize trivially copyable objects.");

    auto size = v.size();
    v.resize(size + sizeof(T));

    std::memcpy(&v[size], &obj, sizeof(T));
}

// Returns the address of the current instruction
uint8_t *GetCurrentAddress(std::vector<uint8_t> &v, uint8_t *base) {
    return static_cast<uint8_t *>(base + v.size());
}

uintptr_t GetDisplacement(uint8_t *base, size_t offset, uint8_t *to) {
    auto from = base + offset + sizeof(to);
    return to - from;
}

bool YAHL::Detour86::WriteStub() {
    std::vector<uint8_t> ins{}; // Assembly instructions to write to the stub

    serialize(ins, AsmIns::Int3);
    
    // Push the detour pointer to the function's arguments
    serialize(ins, AsmIns::Push);
    serialize(ins, this);

    // Call the hooked function
    serialize(ins, AsmIns::Call);
    auto disp = GetDisplacement(stub_, ins.size(), (uint8_t*)hookFunction_);
    serialize(ins, disp);

    // Remove the additional instruction we passed to the hook function (push &detour)
    serialize(ins, AsmIns::Add);
    serialize(ins, AsmIns::Esp);
    serialize(ins, (uint8_t)4);

    // Return at stub to get back to the original caller of the function
    serialize(ins, AsmIns::Ret);

    // This is where we've written the code to call the original function
    originalFunctionCall_ = stub_ + ins.size();

    // Write the instructions to the stub
    std::memcpy(stub_, ins.data(), ins.size());

    return true;
}

bool YAHL::Detour86::WriteTrampoline() {
    std::vector<uint8_t> ins{}; // Assembly instructions to write to the stub
    DWORD old_protect;

    // Make the page writable
    VirtualProtect((void *)originalFunction_, numBytesToHook_, PAGE_EXECUTE_READWRITE, &old_protect);

    // Write the jump to the stub
    serialize(ins, AsmIns::Jmp);
    auto disp = GetDisplacement((uint8_t*)originalFunction_, ins.size(), stub_);
    serialize(ins, disp);

    // If there are additional bytes to write, write nops as padding
    int bytesLeftOver = numBytesToHook_ - ins.size();
    for (int i = 0; i < bytesLeftOver; ++i) {
        serialize(ins, AsmIns::Nop);
    }

    // Write the instructions to the stub
    std::memcpy(originalFunction_, ins.data(), ins.size());

    // Restore original page permissions
    VirtualProtect((void *)originalFunction_, numBytesToHook_, old_protect, &old_protect);

    return true;
}
