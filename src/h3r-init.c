#define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>

#include <stdio.h>

#include "h3api.h"

SEXP h3r_c_h3_version() {
    SEXP out = PROTECT(Rf_allocVector(STRSXP, 1));
    char buf[64];
    sprintf(buf, "%d.%d.%d", H3_VERSION_MAJOR, H3_VERSION_MINOR, H3_VERSION_PATCH);
    SET_STRING_ELT(out, 0, Rf_mkChar(buf));
    UNPROTECT(1);
    return out;
}

static const R_CallMethodDef CallEntries[] = {
  {"h3r_c_h3_version", (DL_FUNC) &h3r_c_h3_version, 0},
  {NULL, NULL, 0}
};

void R_init_h3r(DllInfo *dll) {
  R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
  R_useDynamicSymbols(dll, FALSE);
}
