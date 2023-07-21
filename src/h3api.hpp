#pragma once

#include <array>
#include <charconv>
#include <stdexcept>
#include <string>
#include <system_error>

#include "h3/h3api.h"

H3Index h3_from_str(std::string_view str) {
  H3Index h3_index = H3_NULL;
  if (str.empty()) return h3_index;

  auto [ptr, ec] = std::from_chars(str.begin(), str.end(), h3_index, 16);

  if (ec != std::errc() || *ptr != '\0')
    throw std::invalid_argument("'" + std::string(str) + "' is not a valid h3_index");

  return h3_index;
}

std::string_view h3_to_str(H3Index h3_index, std::array<char, 17>& str) {
  if (h3_index == H3_NULL) return std::string_view();

  auto [ptr, ec] = std::to_chars(str.begin(), str.end(), h3_index, 16);

  if (ec != std::errc()) 
    throw std::invalid_argument(std::make_error_code(ec).message());

  return std::string_view(str.begin(), ptr - str.begin());
}