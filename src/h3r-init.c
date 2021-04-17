#define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>

/* generated by data-raw/call-entries.R */
extern SEXP h3r_c_string_to_h3(SEXP h3_string);
extern SEXP h3r_c_h3_to_string(SEXP h3_index);
extern SEXP h3r_c_h3_version();
static const R_CallMethodDef CallEntries[] = {
    {"h3r_c_string_to_h3", (DL_FUNC) &h3r_c_string_to_h3, 1},
    {"h3r_c_h3_to_string", (DL_FUNC) &h3r_c_h3_to_string, 1},
    {"h3r_c_h3_version", (DL_FUNC) &h3r_c_h3_version, 0},
    {NULL, NULL, 0}
};
/* end generated by data-raw/call-entries.R */

void R_init_h3r(DllInfo *dll) {
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}
