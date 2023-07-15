#pragma once

#include <stdexcept>
#include <string>

#include "h3/h3Index.h"
#include "h3/h3api.h"

H3Index string_to_h3(const char* str) {
  char* end = nullptr;
  errno = 0;
  H3Index h3_index = strtoull(str, &end, 16);

  if (*end != '\0' || errno != 0)
    throw std::invalid_argument(std::string(str) + " is not a valid h3_index");

  return h3_index;
}

uint32_t h3_to_string(H3Index h, char* str, size_t sz) {
  if (sz < 17)
    throw std::length_error(std::string("str size must be >= 17"));

  constexpr char hex[17] = "0123456789abcdef";

  uint32_t out_sz = 0;
  for (uint32_t i = 0, nib = 60; i < 16; ++i, nib -= 4) {
    // value of each nibble as index to hex
    uint32_t c = (h >> nib) & 0xf;
    str[out_sz] = hex[c];

    // drop 0-leading
    out_sz += ((out_sz != 0) | (c != 0));
  }

  // empty string if 0
  str[out_sz] = '\0';
  return out_sz;
}
