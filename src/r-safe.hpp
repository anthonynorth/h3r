#pragma once

#define R_NO_REMAP
#include <Rinternals.h>
#include <stdexcept>
#include "utils.hpp"

/// prevent exceptions from crashing R
template <typename Fn, typename... Params>
decltype(auto) catch_unwind(const Fn& fn, Params&&... params) noexcept {
  try {
    return fn(std::forward<Params>(params)...);
  } catch (const std::exception& err) {
    Rf_error("%s", err.what());
  } catch (...) {
    Rf_error("%s", "C++ error (unknown cause)");
  }

  std::unreachable();
}
