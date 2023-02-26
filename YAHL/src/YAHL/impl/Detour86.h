#pragma once
#include <array>

namespace YAHL {

class Detour86 {
  public:
    Detour86(void *originalFunction, void *hookFunction, size_t numBytesToHook)
        : originalFunction_(originalFunction), hookFunction_(hookFunction), numBytesToHook_(numBytesToHook),
          stub_(nullptr), originalFunctionCall_(nullptr) {}

    bool Enable();

    void CallOriginal();

  private:
    void SaveOriginalBytes();
    bool CreateStub();
    bool WriteStub();
    bool WriteTrampoline();

  private:
    void *originalFunction_ = nullptr;
    void *hookFunction_ = nullptr;
    size_t numBytesToHook_ = 0;

    // Address of our allocated stub
    uint8_t *stub_ = nullptr;
    uint8_t *originalFunctionCall_ = nullptr;

    // Bytes we overwrite with our trampoline
    std::array<uint8_t, 256> originalBytes_{};
};

} // namespace YAHL
