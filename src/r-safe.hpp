#pragma once

#define R_NO_REMAP
#include <Rinternals.h>
#include <stdexcept>
#include "r-interrupt.hpp"
#include "utils.hpp"

/// prevent exceptions from crashing R
template <typename Fn, typename... Params>
decltype(auto) catch_unwind(const Fn& fn, Params&&... params) noexcept {
  const char* msg = nullptr;
  bool interrupt = false;

  try {
    return fn(std::forward<Params>(params)...);
  } catch (const interrupt_error& err) {
    interrupt = true;
  } catch (const std::exception& err) {
    msg = Rf_acopy_string(err.what());
  } catch (...) {
    msg = "C++ error (unknown cause)";
  }

  if (interrupt)
    R_CheckUserInterrupt();
  else
    Rf_error("%s", msg);

  bp::unreachable();
}
