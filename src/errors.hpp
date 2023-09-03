#pragma once

#include <stdio.h>
#include <stdexcept>
#include <string>

template <typename... Params>
struct error : std::runtime_error {
  explicit error(const std::string& msg) : std::runtime_error(msg) {}
  explicit error(std::string_view fmt, Params... params)
      : std::runtime_error(format(fmt, params...)) {}

protected:
  static std::string format(std::string_view fmt, Params... params) {
    char buf[8912];
    size_t size = std::snprintf(buf, sizeof(buf), fmt.data(), params...);
    return {buf, size};
  }
};
