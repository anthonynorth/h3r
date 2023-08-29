#define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>
#include "h3api.hpp"
#include "r-vector.hpp"
#include "wk.hpp"
#include "vctrs.hpp"

struct CellWriter : wk::Handler {
  using Result = wk::Result;

  CellWriter(int res) : res_(res) {}

  virtual Result vector_start(const wk_vector_meta_t* meta) override {
    if (meta->size != WK_VECTOR_SIZE_UNKNOWN) result_.reserve(meta->size);
    return Result::Continue;
  }

  virtual Result null_feature() override {
    result_.push_back(h3_null);
    return Result::Continue;
  }

  virtual Result geometry_start(const wk_meta_t* meta) override {
    using namespace std::string_literals;
    // EMPTY and any set of features that (could) contain a single point work with this
    // handler! (error otherwise)
    if (meta->size != 0 &&
        !is_in(meta->geometry_type, WK_POINT, WK_MULTIPOINT, WK_GEOMETRYCOLLECTION))
      throw std::runtime_error("Can't convert geometry with type "s +
                               std::to_string(meta->geometry_type) + " to h3 cell"s);

    if (meta->size == 0) result_.push_back(h3_null);

    return Result::Continue;
  }

  virtual Result coord(const wk_meta_t* meta, const double* coord) override {
    // FIXME: limit to single coord
    uint64_t cell;
    LatLng point = {degsToRads(coord[1]), degsToRads(coord[0])};
    if (auto err = latLngToCell(&point, res_, &cell); err != H3ErrorCodes::E_SUCCESS)
      throw std::runtime_error("H3 Error");

    result_.push_back(cell == 0 ? h3_null : cell);
    return Result::Continue;
  }

  virtual SEXP vector_end(const wk_vector_meta_t* meta) override {
    result_.set_cls(vctrs_cls::h3_cell);
    return result_;
  }

private:
  int res_;
  vctr<uint64_t, ProtectType::ObjectPreserve> result_;
};

extern "C" SEXP ffi_cell_writer_new(SEXP res_sexp) {
  return catch_unwind([&] {
    int res = Rf_asInteger(res_sexp);
    return wk::HandlerFactory<CellWriter>::create_xptr(new CellWriter(res));
  });
}
