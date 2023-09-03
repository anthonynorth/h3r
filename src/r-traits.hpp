#pragma once

#include <Rinternals.h>
#include <cstdint>
#include <string_view>
#include <type_traits>

// int32
template <typename T>
inline constexpr bool is_int32_v = std::is_same_v<std::remove_cv_t<T>, int32_t>;

// bool
template <typename T>
inline constexpr bool is_bool_v = std::is_same_v<std::remove_cv_t<T>, bool>;

// double
template <typename T>
inline constexpr bool is_double_v = std::is_same_v<std::remove_cv_t<T>, double>;

// int64 & uint64
template <typename T>
struct is_int64 : std::false_type {};

template <>
struct is_int64<int64_t> : std::true_type {};

template <>
struct is_int64<uint64_t> : std::true_type {};

template <typename T>
inline constexpr bool is_int64_v = is_int64<std::remove_cv_t<T>>::value;

// string
template <typename T>
inline constexpr bool is_string_v = std::is_same_v<std::remove_cv_t<T>, std::string_view>;

// list
template <typename T>
inline constexpr bool is_list_v = std::is_same_v<std::remove_cv_t<T>, SEXP>;
