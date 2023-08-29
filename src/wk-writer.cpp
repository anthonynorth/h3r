#define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>
#include "wk-v1.h"
#include "h3/h3api.h"
#include <stdlib.h>
#include <memory.h>

typedef struct {
    SEXP result;
    double* h3_dbl;
    H3Index* h3;
    R_xlen_t result_size;
    R_xlen_t feat_id;
    int has_coord;
    int res;
} h3_point_writer_t;

static inline SEXP h3_point_writer_alloc_result(R_xlen_t size) {
    return Rf_allocVector(REALSXP, size);
}

static inline SEXP h3_point_writer_realloc_result(SEXP result, R_xlen_t new_size) {
    SEXP new_result = PROTECT(h3_point_writer_alloc_result(new_size));

    R_xlen_t size_cpy;
    if (Rf_xlength(result) < new_size) {
        size_cpy = Rf_xlength(result);
    } else {
        size_cpy = new_size;
    }

    memcpy(
        REAL(new_result),
        REAL(result),
        sizeof(double) * size_cpy
    );

    UNPROTECT(1);
    return new_result;
}

static inline void h3_point_writer_append_empty(h3_point_writer_t* writer) {
    if (writer->feat_id >= writer->result_size) {
        SEXP new_result = PROTECT(h3_point_writer_realloc_result(writer->result, writer->result_size * 2 + 1));
        R_ReleaseObject(writer->result);
        writer->result = new_result;
        R_PreserveObject(writer->result);
        UNPROTECT(1);

        writer->result_size = writer->result_size * 2 + 1;
        writer->h3_dbl = REAL(writer->result);
        writer->h3 = (H3Index*) writer->h3_dbl;
    }

    writer->h3_dbl[writer->feat_id] = NA_REAL;
    writer->feat_id++;
}

int h3_point_writer_vector_start(const wk_vector_meta_t* meta, void* handler_data) {
    h3_point_writer_t* writer = (h3_point_writer_t*) handler_data;

    if (writer->result != R_NilValue) {
        Rf_error("Destination vector was already allocated"); // # nocov
    }

    if (meta->size == WK_VECTOR_SIZE_UNKNOWN) {
        writer->result = PROTECT(h3_point_writer_alloc_result(1024));
        writer->result_size = 1024;
    } else {
        writer->result = PROTECT(h3_point_writer_alloc_result(meta->size));
        writer->result_size = meta->size;
    }

    R_PreserveObject(writer->result);
    UNPROTECT(1);

    writer->h3_dbl = REAL(writer->result);
    writer->h3 = (H3Index*) writer->h3_dbl;

    writer->feat_id = 0;

    return WK_CONTINUE;
}

int h3_point_writer_feature_start(const wk_vector_meta_t* meta, R_xlen_t feat_id, void* handler_data) {
    h3_point_writer_t* data = (h3_point_writer_t*) handler_data;
    data->has_coord = 0;
    h3_point_writer_append_empty(data);
    return WK_CONTINUE;
}

int h3_point_writer_geometry_start(const wk_meta_t* meta, uint32_t part_id, void* handler_data) {
    h3_point_writer_t* data = (h3_point_writer_t*) handler_data;

    // EMPTY and any set of features that (could) contain a single point work with this
    // handler! (error otherwise)
    if (meta->size != 0 &&
        meta->geometry_type != WK_POINT &&
        meta->geometry_type != WK_MULTIPOINT &&
        meta->geometry_type != WK_GEOMETRYCOLLECTION) {
        Rf_error(
            "[%d] Can't convert geometry with type '%d' to coordinate",
            data->feat_id + 1,
            meta->geometry_type
        );
    }

    return WK_CONTINUE;
}

int h3_point_writer_coord(const wk_meta_t* meta, const double* coord, uint32_t coord_id, void* handler_data) {
    h3_point_writer_t* data = (h3_point_writer_t*) handler_data;

    if (data->has_coord) {
        Rf_error("[%d] Feature contains more than one coordinate.", data->feat_id);
    } else {
        data->has_coord = 1;
    }

    LatLng latlng = {.lng = degsToRads(coord[0]), .lat = degsToRads(coord[1]) };
    latLngToCell(&latlng, data->res, &data->h3[data->feat_id - 1]);

    return WK_CONTINUE;
}

SEXP h3_point_writer_vector_end(const wk_vector_meta_t* meta, void* handler_data) {
    h3_point_writer_t* data = (h3_point_writer_t*) handler_data;

    R_xlen_t final_size = data->feat_id;
    if (final_size != data->result_size) {
        SEXP new_result = PROTECT(h3_point_writer_realloc_result(data->result, final_size));
        R_ReleaseObject(data->result);
        data->result = new_result;
        R_PreserveObject(data->result);
        UNPROTECT(1);
    }

    SEXP index_class = PROTECT(Rf_allocVector(STRSXP, 2));
    SET_STRING_ELT(index_class, 0, Rf_mkChar("h3_index"));
    SET_STRING_ELT(index_class, 1, Rf_mkChar("vctrs_vctr"));
    Rf_setAttrib(data->result, R_ClassSymbol, index_class);
    UNPROTECT(1);

    return data->result;
}

void h3_point_writer_deinitialize(void* handler_data) {
    h3_point_writer_t* data = (h3_point_writer_t*) handler_data;
    if (data->result != R_NilValue) {
        R_ReleaseObject(data->result);
        data->result = R_NilValue;
    }
}

void h3_point_writer_finalize(void* handler_data) {
    h3_point_writer_t* data = (h3_point_writer_t*) handler_data;
    if (data != NULL) {
        free(data);
    }
}

SEXP h3r_c_point_writer_new(SEXP res_sexp) {
    int res = INTEGER(res_sexp)[0];
    if (res > 15 || res < 0) {
        Rf_error("`res` must be between 0 and 15");
    }

    wk_handler_t* handler = wk_handler_create();

    handler->vector_start = &h3_point_writer_vector_start;
    handler->feature_start = &h3_point_writer_feature_start;
    handler->geometry_start = &h3_point_writer_geometry_start;
    handler->coord = &h3_point_writer_coord;
    handler->vector_end = &h3_point_writer_vector_end;
    handler->deinitialize = &h3_point_writer_deinitialize;
    handler->finalizer = &h3_point_writer_finalize;

    h3_point_writer_t* data = (h3_point_writer_t*) malloc(sizeof(h3_point_writer_t));
    if (data == NULL) {
        wk_handler_destroy(handler); // # nocov
        Rf_error("Failed to alloc handler data"); // # nocov
    }

    data->feat_id = 0;
    data->has_coord = 0;
    data->result = R_NilValue;
    data->res = res;

    handler->handler_data = data;

    SEXP xptr = wk_handler_create_xptr(handler, R_NilValue, R_NilValue);
    return xptr;
}
