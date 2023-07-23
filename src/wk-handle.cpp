#define R_NO_REMAP

#include <algorithm>
#include <vector>
#include "h3api.hpp"
#include "r-safe.hpp"
#include "r-vector.hpp"
#include "wk.hpp"

// h3index vector reader
template <typename T>
struct Reader {
  using Result = typename wk::Result;

  Reader(wk::NextHandler next) : next_(next) {
    WK_VECTOR_META_RESET(vector_meta_, WK_GEOMETRY);
    WK_META_RESET(meta_, WK_GEOMETRY);
  }

  virtual ~Reader() {}

  virtual Result read_feature(const T& feature) { throw std::logic_error("Not implemented"); }

  virtual SEXP read_features(const vctr_view<T>& features) {
    vector_meta_.size = features.size();
    next_.vector_start(&vector_meta_);

    for (auto feature : features) {
      next_.feature_start(&vector_meta_);
      read_feature(feature);
      next_.feature_end(&vector_meta_);
    }

    return next_.vector_end(&vector_meta_);
  }

  virtual Result read_coord(const LatLng& point) {
    double coord[2];
    coord[0] = radsToDegs(point.lng);
    coord[1] = radsToDegs(point.lat);
    return next_.coord(&meta_, coord);
  }

protected:
  wk::NextHandler next_;
  wk_vector_meta_t vector_meta_;
  wk_meta_t meta_;
};

// read cell centroids
struct CellCentroidReader : Reader<uint64_t> {
  CellCentroidReader(wk::NextHandler next) : Reader(next) {
    WK_VECTOR_META_RESET(vector_meta_, WK_POINT);
    WK_META_RESET(meta_, WK_POINT);
  }

  Result read_feature(const uint64_t& cell) override {
    LatLng point;
    cellToLatLng(cell, &point);

    meta_.size = 1;
    next_.geometry_start(&meta_);
    read_coord(point);
    return next_.geometry_end(&meta_);
  }
};

// read cell polygons
struct CellPolygonReader : Reader<uint64_t> {
  CellPolygonReader(wk::NextHandler next) : Reader(next) {
    WK_VECTOR_META_RESET(vector_meta_, WK_POLYGON);
    WK_META_RESET(meta_, WK_POLYGON);
  }

  Result read_feature(const uint64_t& cell) override {
    CellBoundary boundary;
    cellToBoundary(cell, &boundary);

    meta_.size = 1;
    next_.geometry_start(&meta_);
    read_coords(boundary);
    return next_.geometry_end(&meta_);
  }

  Result read_coords(const CellBoundary& boundary) {
    uint32_t ring_size = boundary.numVerts + 1;
    next_.ring_start(&meta_, ring_size);

    for (int i = 0; i < boundary.numVerts; i++) {
      read_coord(boundary.verts[i]);
    }

    // close the polygon
    read_coord(boundary.verts[0]);

    return next_.ring_end(&meta_, ring_size);
  }
};

SEXP handle_cell(SEXP data, wk_handler_t* handler) {
  return catch_unwind([&] {
    vctr_view<uint64_t> cells = VECTOR_ELT(data, 0);
    auto type = Rf_asInteger(VECTOR_ELT(data, 1));

    if (type == 1) {
      CellPolygonReader reader(handler);
      return reader.read_features(cells);
    }

    CellCentroidReader reader(handler);
    return reader.read_features(cells);
  });
}

extern "C" SEXP ffi_handle_cell(SEXP data, SEXP handler_xptr) {
  return wk_handler_run_xptr(&handle_cell, data, handler_xptr);
}