#define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>

#include <stdint.h>
#include <stdio.h>

#include "h3/h3api.h"

SEXP h3r_c_string_to_h3(SEXP h3_string) {
    R_xlen_t size = Rf_xlength(h3_string);
    SEXP h3_index = PROTECT(Rf_allocVector(REALSXP, size));
    double* result_dbl = REAL(h3_index);
    H3Index* h3 = (H3Index*) REAL(h3_index);

    SEXP item;
    for (R_xlen_t i = 0; i < size; i++) {
        item = STRING_ELT(h3_string, i);
        if (item == NA_STRING) {
            result_dbl[i] = NA_REAL;
        } else {
            h3[i] = stringToH3(CHAR(item));
        }
    }

    UNPROTECT(1);
    return h3_index;
}

SEXP h3r_c_h3_to_string(SEXP h3_index) {
    R_xlen_t size = Rf_xlength(h3_index);
    double* h3_dbl = REAL(h3_index);
    H3Index* h3 = (H3Index*) REAL(h3_index);

    SEXP result = PROTECT(Rf_allocVector(STRSXP, size));
    char buf[17];
    for (R_xlen_t i = 0; i < size; i++) {
        if (ISNA(h3_dbl[i])) {
            SET_STRING_ELT(result, i, NA_STRING);
        } else {
            h3ToString(h3[i], buf, sizeof(buf));
            SET_STRING_ELT(result, i, Rf_mkChar(buf));
        }
    }

    UNPROTECT(1);
    return result;
}
