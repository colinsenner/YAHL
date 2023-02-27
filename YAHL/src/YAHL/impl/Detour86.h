#pragma once
#include <Windows.h>
#include <functional>
#include <iostream>
#include <vector>

namespace YAHL {

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

template <typename T> void serialize(std::vector<uint8_t> &v, const T &obj) {
    static_assert(std::is_trivially_copyable<T>::value, "Can only serialize trivially copyable objects.");

    auto size = v.size();
    v.resize(size + sizeof(T));

    std::memcpy(&v[size], &obj, sizeof(T));
}

class Detour86 {
  public:
    Detour86(void *originalFunction, void *hookFunction, size_t numBytesToHook)
        : oFunc_(originalFunction), hFunc_(hookFunction), numBytesToHook_(numBytesToHook),
          stub_(nullptr) {}

    bool Enable() {
        if (enabled_) {
            return true;
        }

        // Ensure there's enough bytes to hook
        if (numBytesToHook_ < 5) {
            return false;
        }

        SaveOriginalBytes();

        if (!CreateStub()) {
            return false;
        }

        WriteStub();

        if (!WriteTrampoline()) {
            DestroyStub();
            return false;
        }

        enabled_ = true;
        return true;
    }

    bool Disable() {
        if (!enabled_) {
            return true;
        }

        if (!RestoreOriginalBytes()) {
            return false;
        }

        if (!DestroyStub()) {
            return false;
        }

        enabled_ = false;
        return true;
    }

    template <typename... Args> auto CallOriginal(Args &&...args) {
        using Callable = void (*)(Args...); // Create a callable type to cast the address we stored

        // Call the address at the end of our stub where we copied the original bytes
        return std::invoke(std::forward<Callable>((Callable)originalBytesAddress_), std::forward<Args>(args)...);
    }

  private:
    void SaveOriginalBytes() {
        originalBytes_.clear();
        originalBytes_.reserve(numBytesToHook_);

        for (size_t i = 0; i < numBytesToHook_; ++i) {
            auto byte = *(uint8_t *)((uint8_t *)oFunc_ + i);
            originalBytes_.push_back(byte);
        }
    }

    bool RestoreOriginalBytes() noexcept {
        DWORD old_protect;

        // Make the page writable
        auto success = VirtualProtect((void *)oFunc_, numBytesToHook_, PAGE_EXECUTE_READWRITE, &old_protect);

        if (!success) {
            return false;
        }

        std::memcpy(oFunc_, originalBytes_.data(), originalBytes_.size());

        // Restore original page permissions
        success = VirtualProtect((void *)oFunc_, numBytesToHook_, old_protect, &old_protect);

        return success != 0;
    }

    bool CreateStub() {
        stub_ = (uint8_t *)VirtualAlloc(NULL, stubSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        return stub_;
    }

    bool DestroyStub() {
        auto success = VirtualFree(stub_, 0, MEM_RELEASE);

        if (!success) {
            return false;
        }

        stub_ = nullptr;
        return true;
    }

    void WriteStub() {
        std::vector<uint8_t> ins{}; // Assembly instructions to write to the stub

        // serialize(ins, AsmIns::Int3);

        // Push an additional argument to our hook
        // Each hook function gets a Detour* as it's first argument
        serialize(ins, AsmIns::Push);
        serialize(ins, this);

        // Call the hooked function
        serialize(ins, AsmIns::Call);
        auto disp = GetDisplacement(stub_, ins.size(), (uint8_t *)hFunc_);
        serialize(ins, disp);

        // Remove the additional instruction we passed to the hook function (push &detour)
        serialize(ins, AsmIns::Add);
        serialize(ins, AsmIns::Esp);
        serialize(ins, (uint8_t)4);

        // Return at stub to get back to the original caller of the function
        serialize(ins, AsmIns::Ret);

        // Store the address of this location so when we invoke CallOriginal(...) it calls here
        // To avoid infinite loops when the hook calls the original code
        originalBytesAddress_ = (stub_ + ins.size());

        // Write the original bytes at the end of the stub,
        for (const auto &byte : originalBytes_) {
            serialize(ins, byte);
        }

        // Jump back to the original function
        serialize(ins, AsmIns::Jmp);
        disp = GetDisplacement(stub_, ins.size(), (uint8_t *)oFunc_ + numBytesToHook_);
        serialize(ins, disp);

        // Write the instructions to the stub
        std::memcpy(stub_, ins.data(), ins.size());
    }

    bool WriteTrampoline() {
        std::vector<uint8_t> ins{}; // Assembly instructions to write to the stub
        DWORD old_protect;

        // Make the page writable
        auto success = VirtualProtect((void *)oFunc_, numBytesToHook_, PAGE_EXECUTE_READWRITE, &old_protect);

        if (!success) {
            return false;
        }

        // Write the jump to the stub
        serialize(ins, AsmIns::Jmp);
        auto disp = GetDisplacement((uint8_t *)oFunc_, ins.size(), stub_);
        serialize(ins, disp);

        // If there are additional bytes to write, write nops as padding
        int bytesLeftOver = numBytesToHook_ - ins.size();
        for (int i = 0; i < bytesLeftOver; ++i) {
            serialize(ins, AsmIns::Nop);
        }

        // Write the instructions to the stub
        std::memcpy(oFunc_, ins.data(), ins.size());

        // Restore original page permissions
        success = VirtualProtect((void *)oFunc_, numBytesToHook_, old_protect, &old_protect);

        if (!success) {
            return false;
        }

        return true;
    }

    uint8_t *GetCurrentAddress(std::vector<uint8_t> &v, uint8_t *base) {
        return static_cast<uint8_t *>(base + v.size());
    }

    uintptr_t GetDisplacement(uint8_t *base, size_t offset, uint8_t *to) {
        auto from = base + offset + sizeof(to);
        return to - from;
    }

  private:
    void *oFunc_ = nullptr;
    void *hFunc_ = nullptr;
    size_t numBytesToHook_ = 0;

    bool enabled_ = false;

    // Address of our allocated stub
    uint8_t *stub_ = nullptr;

    // Bytes we overwrite with our trampoline
    std::vector<uint8_t> originalBytes_{};

    // We store the prologue bytes of the original function at the end of our stub
    // To avoid a circular loop where inside our hook we call the original function (which would call our hook again)
    // we can store the address and cast this to a callable so we can std::invoke it
    void *originalBytesAddress_ = nullptr;
};

} // namespace YAHL
