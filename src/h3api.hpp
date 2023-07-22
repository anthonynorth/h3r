#pragma once

#define R_NO_REMAP
#include <R.h>
#include <array>
#include <charconv>
#include <stdexcept>
#include <string>
#include <system_error>
#include "h3/h3api.h"
#include "utils.hpp"

const H3Index h3_null = bit_cast<H3Index>(NA_REAL);

bool h3_is_null(H3Index h3_index) {
  return h3_index == h3_null;
}

H3Index h3_from_str(std::string_view str) {
  if (str.empty()) return h3_null;

  H3Index h3_index = h3_null;
  auto [ptr, ec] = std::from_chars(str.begin(), str.end(), h3_index, 16);

  if (ec != std::errc() || *ptr != '\0')
    throw std::invalid_argument("'" + std::string(str) + "' is not a valid h3_index");

  return h3_index;
}

std::string_view h3_to_str(H3Index h3_index, std::array<char, 17>& str) {
  if (h3_is_null(h3_index)) return std::string_view();

  auto [ptr, ec] = std::to_chars(str.begin(), str.end(), h3_index, 16);

  if (ec != std::errc()) 
    throw std::invalid_argument(std::make_error_code(ec).message());

  return std::string_view(str.begin(), ptr - str.begin());
}