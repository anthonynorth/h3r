#pragma once

#define R_NO_REMAP
#include <Rinternals.h>
#include <stdexcept>

/// prevent exceptions from crashing R
template <typename Fn, typename... Params>
decltype(auto) catch_unwind(const Fn& fn, Params&&... params) noexcept {
  try {
    return fn(std::forward<Params>(params)...);
  } catch (const std::exception& err) {
    Rf_errorcall(R_NilValue, err.what());
  } catch (...) {
    Rf_errorcall(R_NilValue, "C++ error (unknown cause)");
  }

  // unreachable
}