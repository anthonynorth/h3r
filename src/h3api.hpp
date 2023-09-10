#pragma once

#define R_NO_REMAP
#include <R.h>
#include <array>
#include <charconv>
#include <stdexcept>
#include <string>
#include <system_error>
#include <unordered_set>
#include "errors.hpp"
#include "geom.hpp"
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

constexpr double pentagon_radius(int res) {
  // pentagon max haversine(centroid, vertex)
  // clang-format off
  constexpr double radii[16] = {
      0.1627467775419132,     0.05531000215848966,    0.021576562318774757,   0.007640139412730925,
      0.0030479908472205923,  0.0010861879312414082,  0.0004347266499694732,  0.0001550625768904632,
      6.208951215426715e-05,  2.214961110272488e-05,  8.869638583639624e-06,  3.1641855561633976e-06,
      1.2670852730677077e-06, 4.5202559844899493e-07, 1.8101206095593024e-07, 6.457506774604356e-08};
  // clang-format on

  if (res < 0 || res > 15) throw std::range_error(fmt_error(E_RES_DOMAIN));
  return radii[res];
}

/// find cell at `res` intersecting `n_vector`
inline H3Error nvector_to_cell(const NVector& n_vector, int res, uint64_t* cell) {
  Coord coord = n_vector.to_coord();
  return latLngToCell(&coord, res, cell);
}

/// find cells at `res` intersecting `arc` by sampling
/// NOTE: sampling doesn't include `arc.end`
inline H3Error arc_to_cells(const Arc& arc, int res, std::unordered_set<uint64_t>& cells) {
  // distance between arc samples at most 1/3 pentagon_radius apart
  const double n_steps = std::ceil(3 * arc.length() / pentagon_radius(res));

  for (uint64_t i = 0; i < n_steps; i++) {
    NVector vec = arc.lerp(i / n_steps);
    Coord coord = vec.to_coord();

    uint64_t cell;
    if (auto err = latLngToCell(&coord, res, &cell); err != E_SUCCESS) return err;
    cells.insert(cell);
  }

  return E_SUCCESS;
}

/// find cells at `res` intersecting `arc_string`
inline H3Error arcstring_to_cells(const ArcString& arc_string, int res, std::unordered_set<uint64_t>& cells) {
  for (auto arc : arc_string) {
    if (auto err = arc_to_cells(arc, res, cells); err != E_SUCCESS) return err;
  }

  // last coord hasn't been checked
  uint64_t cell;
  if (auto err = nvector_to_cell(arc_string.coords.back(), res, &cell); err != E_SUCCESS) return err;
  cells.insert(cell);

  return E_SUCCESS;
}

};  // namespace h3