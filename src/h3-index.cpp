#define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>
#include <algorithm>
#include <array>
#include <string>

#include "h3api.hpp"
#include "r-safe.hpp"
#include "r-vector.hpp"

extern "C" SEXP ffi_string_to_h3(SEXP strings_sxp) {
  return catch_unwind([&] {
    vctr_view<std::string_view> strings = strings_sxp;
    vctr<H3Index> h3_indexes(strings.size());

    std::transform(strings.begin(), strings.end(), h3_indexes.begin(), h3_from_str);
    return h3_indexes;
  });
}

extern "C" SEXP ffi_h3_to_string(SEXP h3_indexes_sxp) {
  return catch_unwind([&] {
    vctr_view<H3Index> h3_indexes = h3_indexes_sxp;
    vctr<std::string_view> strings(h3_indexes.size());

    std::array<char, 17> buf;
    std::transform(h3_indexes.begin(), h3_indexes.end(), strings.begin(),
                   [&buf](auto h3) { return h3_to_str(h3, buf); });
    return strings;
  });
}