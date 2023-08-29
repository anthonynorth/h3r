#pragma once

#include <string_view>

namespace vctrs_cls {
using std::string_view_literals::operator""sv;

// h3_cell
constexpr std::initializer_list<std::string_view> h3_cell = {"h3_cell"sv, "h3_index"sv,
                                                             "vctrs_vctr"sv};
// h3_directed_edge
constexpr std::initializer_list<std::string_view> h3_directed_edge = {"h3_directed_edge"sv,
                                                                      "h3_index"sv, "vctrs_vctr"sv};
// h3_vertex
constexpr std::initializer_list<std::string_view> h3_vertex = {"h3_vertex"sv, "h3_index"sv,
                                                               "vctrs_vctr"sv};
// list_of
constexpr std::initializer_list<std::string_view> list_of = {"vctrs_list_of"sv, "vctrs_vctr"sv,
                                                             "list"sv};
};  // namespace vctrs_cls