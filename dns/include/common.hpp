#pragma once

#include <cstddef>
#include <cstdint>

namespace microdns
{

inline constexpr size_t MAX_DNS_LEN = 512;
inline constexpr int MAX_STUB_PENDING = 20;

typedef uint16_t QueryID;

} // namespace microdns
