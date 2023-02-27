#pragma once
#include "impl/Detour86.h"

namespace YAHL {

#ifdef WIN32 // 32-bit
using Detour = YAHL::Impl::Detour86;
#else // 64-bit

#endif

} // namespace YAHL