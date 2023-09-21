#pragma once

#define R_NO_REMAP
#include <Rinternals.h>
#include <stdexcept>
#include "utils.hpp"

/// prevent exceptions from crashing R
template <typename Fn, typename... Params>
decltype(auto) catch_unwind(const Fn& fn, Params&&... params) noexcept {
  const char* msg = nullptr;

  try {
    return fn(std::forward<Params>(params)...);
  } catch (const std::exception& err) {
    msg = Rf_acopy_string(err.what());
  } catch (...) {
    msg = "C++ error (unknown cause)";
  }

  Rf_error("%s", msg);
  bp::unreachable();
}
