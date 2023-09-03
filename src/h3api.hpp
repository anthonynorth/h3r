#pragma once

#define R_NO_REMAP
#include <R.h>
#include <array>
#include <charconv>
#include <stdexcept>
#include <string>
#include <system_error>
#include "errors.hpp"
#include "h3/h3api.h"
#include "utils.hpp"

const H3Index h3_null = bp::bit_cast<H3Index>(NA_REAL);

inline bool h3_is_null(H3Index h3_index) {
  return h3_index == h3_null;
}

inline H3Index h3_from_str(std::string_view str) {
  if (str.empty()) return h3_null;

  H3Index h3_index = h3_null;
  auto [ptr, ec] = std::from_chars(str.begin(), str.end(), h3_index, 16);

  if (ec != std::errc() || *ptr != '\0')
    throw std::invalid_argument("'" + std::string(str) + "' is not a valid h3_index");

  return h3_index;
}

inline std::string_view h3_to_str(H3Index h3_index, std::array<char, 17>& str) {
  if (h3_is_null(h3_index)) return std::string_view();

  auto [ptr, ec] = std::to_chars(str.begin(), str.end(), h3_index, 16);

  if (ec != std::errc())
    throw std::invalid_argument(std::make_error_code(ec).message());

  return std::string_view(str.begin(), ptr - str.begin());
}

namespace h3 {

template <typename T>
const char* fmt_error(T err) {
  static_assert(std::is_same_v<T, H3ErrorCodes> || std::is_same_v<T, H3Error>);

  switch (err) {
    case H3ErrorCodes::E_FAILED:
      return "The operation failed but a more specific error is not available";
    case H3ErrorCodes::E_DOMAIN:
      return "Argument was outside of acceptable range";
    case H3ErrorCodes::E_LATLNG_DOMAIN:
      return "Latitude or longitude arguments were outside of acceptable range";
    case H3ErrorCodes::E_RES_DOMAIN:
      return "Resolution argument was outside of acceptable range";
    case H3ErrorCodes::E_CELL_INVALID:
      return "Cell argument was not valid";
    case H3ErrorCodes::E_DIR_EDGE_INVALID:
      return "Directed edge argument was not valid";
    case H3ErrorCodes::E_UNDIR_EDGE_INVALID:
      return "Undirected edge argument was not valid";
    case H3ErrorCodes::E_VERTEX_INVALID:
      return "Vertex argument was not valid";
    case H3ErrorCodes::E_PENTAGON:
      return "Pentagon distortion was encountered";
    case H3ErrorCodes::E_DUPLICATE_INPUT:
      return "Duplicate input was encountered";
    case H3ErrorCodes::E_NOT_NEIGHBORS:
      return "Cell arguments were not neighbours";
    case H3ErrorCodes::E_RES_MISMATCH:
      return "Cell arguments had incompatible resolutions";
    case H3ErrorCodes::E_MEMORY_ALLOC:
      return "Memory allocation failed";
    case H3ErrorCodes::E_MEMORY_BOUNDS:
      return "Bounds of provided memory were not large enough";
    case H3ErrorCodes::E_OPTION_INVALID:
      return "Mode or flags argument was not valid";
    default:
      return "Unknown error";
  }
}

};  // namespace h3

