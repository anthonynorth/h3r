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

int h3_to_string(H3Index h3_index, char* str, size_t sz) {
  if (sz < 17)
    throw std::length_error(std::string("str size must be >= 17"));

  constexpr char hex[17] = "0123456789abcdef";
  int out_sz = 16;

  // leading 0s are dropped
  for (int offset = 60; (offset >= 0) && (h3_index >> offset) == 0; offset -= 4) {
    --out_sz;
  }
  
  // write in reverse
  for (int i = out_sz - 1; i >= 0; --i, h3_index >>= 4) {
    // value of each nibble as index to hex
    str[i] = hex[h3_index & 0xf];
  }

  str[out_sz + 1] = '\0';
  return out_sz;
}
