#pragma once

#include <cstring>
#include <type_traits>

template <typename...>
inline constexpr bool unsupported = false;

template <typename T, typename... Ts>
constexpr bool is_in(T value, Ts... values) {
  return ((values == value) || ...);
}

namespace bp {
// clang-format off

// backport from c++20
template <class To, class From>
[[nodiscard]]
constexpr std::enable_if_t<
  sizeof(To) == sizeof(From) &&
  std::is_trivially_copyable_v<From> &&
  std::is_trivially_copyable_v<To>,
  To>
bit_cast(const From& src) noexcept {
  static_assert(std::is_trivially_constructible_v<To>,
    "This implementation additionally requires "
    "destination type to be trivially constructible");

  To dst;
  std::memcpy(&dst, &src, sizeof(To));
  return dst;
}

// backport from c++23
[[noreturn]] inline void unreachable() {
#if defined __GNUC__  // GCC, Clang, ICC
  __builtin_unreachable();
#elif defined _MSC_VER  // MSVC
  __assume(false);
#endif
}

// clang-format on
};  // namespace bp
