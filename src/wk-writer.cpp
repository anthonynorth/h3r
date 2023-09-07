#define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>
#include <algorithm>
#include <unordered_set>
#include <vector>
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
        !is_in(meta->geometry_type, WK_POINT, WK_MULTIPOINT, WK_GEOMETRYCOLLECTION))
      throw error("[%i] Cannot convert geometry type '%s' to h3_cell", cur_feat(),
                  wk::fmt_geometry_type(meta->geometry_type));

    return Result::Continue;
  }

  Result coord(const wk_meta_t* meta, const double* coord) override {
    if (++coord_id_ != 0) throw error("[%i] Feature must contain 0 or 1 coordinate", cur_feat());

    uint64_t cell;
    LatLng point = {degsToRads(coord[1]), degsToRads(coord[0])};
    if (auto err = latLngToCell(&point, res_, &cell); err != E_SUCCESS)
      throw error("[%i] H3 Error: %s", cur_feat(), h3::fmt_error(err));

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

  uint64_t cur_feat() const { return feat_id_ + 1; }
};

struct ListOfCellWriter : wk::Handler {
  using Result = wk::Result;

  ListOfCellWriter(int res) : res_(res) {}

  Result vector_start(const wk_vector_meta_t* meta) override {
    if (meta->size != WK_VECTOR_SIZE_UNKNOWN) result_.reserve(meta->size);
    return Result::Continue;
  }

  Result feature_start(const wk_vector_meta_t* meta) override {
    ++feat_id_;
    cells_.clear();
    return Result::Continue;
  }

  Result geometry_start(const wk_meta_t* meta) override {
    coords_.clear();
    return Result::Continue;
  }

  Result coord(const wk_meta_t* meta, const double* coord) override {
    coords_.push_back({degsToRads(coord[1]), degsToRads(coord[0])});
    return Result::Continue;
  }

  Result geometry_end(const wk_meta_t* meta) override {
    if (coords_.empty()) return Result::Continue;

    if (meta->geometry_type == WK_POINT) {
      uint64_t cell;

      if (auto err = latLngToCell(&coords_.back(), res_, &cell); err != E_SUCCESS)
        throw error("[%i] H3 Error: %s", cur_feat(), h3::fmt_error(err));

      cells_.insert(cell);
      return Result::Continue;
    }
  }

  Result feature_end(const wk_vector_meta_t* meta) override {
    vctr<uint64_t> feature_cells(cells_.size());
    feature_cells.set_cls(vctrs_cls::h3_cell);
    std::copy(cells_.begin(), cells_.end(), feature_cells.begin());
    result_.push_back(feature_cells);

    return Result::Continue;
  }

  SEXP vector_end(const wk_vector_meta_t* meta) override {
    result_.set_cls(vctrs_cls::list_of);

    vctr<uint64_t> ptype;
    ptype.set_cls(vctrs_cls::h3_cell);
    result_.set_ptype(ptype);

    return result_;
  }

private:
  int res_;
  int64_t feat_id_ = -1;
  std::vector<LatLng> coords_;
  std::unordered_set<uint64_t> cells_;
  vctr<SEXP, ProtectType::ObjectPreserve> result_;

  int64_t cur_feat() const { return feat_id_ + 1; }
};

extern "C" SEXP ffi_cell_writer_new(SEXP res_sexp) {
  return catch_unwind([&] {
    int res = Rf_asInteger(res_sexp);
    return wk::HandlerFactory<CellWriter>::create_xptr(new CellWriter(res));
  });
}

extern "C" SEXP ffi_listof_cell_writer_new(SEXP res_sexp) {
  return catch_unwind([&] {
    int res = Rf_asInteger(res_sexp);
    return wk::HandlerFactory<ListOfCellWriter>::create_xptr(new ListOfCellWriter(res));
  });
}
