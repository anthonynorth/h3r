#define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>
#include "h3api.hpp"
#include "r-vector.hpp"
#include "vctrs.hpp"
#include "wk.hpp"

struct CellWriter : wk::Handler {
  using Result = wk::Result;

  CellWriter(int res) : res_(res) {}

  Result vector_start(const wk_vector_meta_t* meta) override {
    if (meta->size != WK_VECTOR_SIZE_UNKNOWN) result_.reserve(meta->size);
    return Result::Continue;
  }

  Result feature_start(const wk_vector_meta_t* meta) override {
    ++feat_id_;
    coord_id_ = -1;
    return Result::Continue;
  }

  Result geometry_start(const wk_meta_t* meta) override {
    // EMPTY and any set of features that (could) contain a single point work with this
    // handler! (error otherwise)
    if (meta->size != 0 &&
        !is_in(meta->geometry_type, WK_POINT, WK_MULTIPOINT, WK_GEOMETRYCOLLECTION)) {
      auto msg = "[" + std::to_string(feat_id_ + 1) + "] Can't convert geometry with type " +
                 std::to_string(meta->geometry_type) + " to cell";
      throw std::runtime_error(msg);
    }

    return Result::Continue;
  }

  Result coord(const wk_meta_t* meta, const double* coord) override {
    if (++coord_id_ != 0) {
      auto msg =
          "[" + std::to_string(feat_id_ + 1) + "] Feature contains more than one coordinate.";
      throw std::runtime_error(msg);
    }

    uint64_t cell;
    LatLng point = {degsToRads(coord[1]), degsToRads(coord[0])};
    if (auto err = latLngToCell(&point, res_, &cell); err != H3ErrorCodes::E_SUCCESS)
      throw std::runtime_error("H3 Error");

    result_.push_back(cell);
    return Result::Continue;
  }

  Result feature_end(const wk_vector_meta_t* meta) override {
    // didn't write a coordinate?
    if (coord_id_ == -1) result_.push_back(h3_null);
    return Result::Continue;
  }

  SEXP vector_end(const wk_vector_meta_t* meta) override {
    result_.set_cls(vctrs_cls::h3_cell);
    return result_;
  }

private:
  int res_;
  uint64_t feat_id_ = -1;
  uint32_t coord_id_ = -1;
  vctr<uint64_t, ProtectType::ObjectPreserve> result_;
};

extern "C" SEXP ffi_cell_writer_new(SEXP res_sexp) {
  return catch_unwind([&] {
    int res = Rf_asInteger(res_sexp);
    return wk::HandlerFactory<CellWriter>::create_xptr(new CellWriter(res));
  });
}
