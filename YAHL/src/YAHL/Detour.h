#pragma once
#include "impl/Detour86.h"

namespace YAHL {

#ifdef WIN32 // 32-bit
template <class T> 
using Detour = YAHL::Detour86<T>;
#else // 64-bit

#endif

} // namespace YAHL