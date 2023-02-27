#pragma once
#include <stdint.h>
#include <vector>

namespace YAHL::Impl {

const uint32_t stubSize = 256;

template <typename T> void serialize(std::vector<uint8_t> &v, const T &obj) {
    static_assert(std::is_trivially_copyable<T>::value, "Can only serialize trivially copyable objects.");

    auto size = v.size();
    v.resize(size + sizeof(T));

    std::memcpy(&v[size], &obj, sizeof(T));
}

} // namespace YAHL::Impl