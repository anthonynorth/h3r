#define R_NO_REMAP

#include <algorithm>
#include <vector>
#include "h3api.hpp"
#include "r-safe.hpp"
#include "r-vector.hpp"
#include "wk.hpp"
#include "errors.hpp"

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

    auto res = next_.vector_start(&vector_meta_);
    if (res != Result::Continue) return next_.vector_end(&vector_meta_);

    for (auto feature : features) {
      if (res == Result::Abort) break;

      ++feat_id_;
      res = next_.feature_start(&vector_meta_);
      if (res != Result::Continue) continue;

      res = read_feature(feature);
      if (res != Result::Continue) continue;

      res = next_.feature_end(&vector_meta_);
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
  int64_t feat_id_ = -1;

  int64_t cur_feat() const { return feat_id_ + 1; }
};

// read cell centroids
struct CellCentroidReader : Reader<uint64_t> {
  CellCentroidReader(wk::NextHandler next) : Reader(next) {
    WK_VECTOR_META_RESET(vector_meta_, WK_POINT);
    WK_META_RESET(meta_, WK_POINT);
  }

  Result read_feature(const uint64_t& cell) override {
    bool not_null = !h3_is_null(cell);
    meta_.size = not_null;

    auto res = next_.geometry_start(&meta_);
    if (res != Result::Continue) return res;

    if (not_null) {
      LatLng point;
      if (auto err = cellToLatLng(cell, &point); err != H3ErrorCodes::E_SUCCESS)
        throw error("[%i] H3 Error: %s", cur_feat(), h3::fmt_error(err));

      res = read_coord(point);
      if (res != Result::Continue) return res;
    }

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
    bool not_null = !h3_is_null(cell);
    meta_.size = not_null;

    auto res = next_.geometry_start(&meta_);
    if (res != Result::Continue) return res;

    if (not_null) {
      CellBoundary boundary;
      if (auto err = cellToBoundary(cell, &boundary); err != H3ErrorCodes::E_SUCCESS)
        throw error("[%i] H3 Error: %s", cur_feat(), h3::fmt_error(err));

      res = read_coords(boundary);
      if (res != Result::Continue) return res;
    }

    return next_.geometry_end(&meta_);
  }

  Result read_coords(const CellBoundary& boundary) {
    uint32_t ring_size = boundary.numVerts + 1;

    auto res = next_.ring_start(&meta_, ring_size);
    if (res != Result::Continue) return res;

    for (int i = 0; i < boundary.numVerts; i++) {
      res = read_coord(boundary.verts[i]);
      if (res != Result::Continue) return res;
    }

    // close the polygon
    res = read_coord(boundary.verts[0]);
    if (res != Result::Continue) return res;

    return next_.ring_end(&meta_, ring_size);
  }
};

// read directed edges
struct DirectedEdgeReader : Reader<uint64_t> {
  DirectedEdgeReader(wk::NextHandler next) : Reader(next) {
    WK_VECTOR_META_RESET(vector_meta_, WK_LINESTRING);
    WK_META_RESET(meta_, WK_LINESTRING);
  }

  Result read_feature(const uint64_t& edge) override {
    bool not_null = !h3_is_null(edge);
    meta_.size = not_null;

    auto res = next_.geometry_start(&meta_);
    if (res != Result::Continue) return res;

    if (not_null) {
      CellBoundary boundary;
      if (auto err = directedEdgeToBoundary(edge, &boundary); err != H3ErrorCodes::E_SUCCESS)
        throw error("[%i] H3 Error: %s", cur_feat(), h3::fmt_error(err));

      res = read_coord(boundary.verts[0]);
      if (res != Result::Continue) return res;

      res = read_coord(boundary.verts[1]);
      if (res != Result::Continue) return res;
    }

    return next_.geometry_end(&meta_);
  }
};

// read vertexes
struct VertexReader : Reader<uint64_t> {
  VertexReader(wk::NextHandler next) : Reader(next) {
    WK_VECTOR_META_RESET(vector_meta_, WK_POINT);
    WK_META_RESET(meta_, WK_POINT);
  }

  Result read_feature(const uint64_t& cell) override {
    bool not_null = !h3_is_null(cell);
    meta_.size = not_null;

    auto res = next_.geometry_start(&meta_);
    if (res != Result::Continue) return res;

    if (not_null) {
      LatLng point;
      if (auto err = vertexToLatLng(cell, &point); err != H3ErrorCodes::E_SUCCESS)
        throw error("[%i] H3 Error: %s", cur_feat(), h3::fmt_error(err));

      auto res = read_coord(point);
      if (res != Result::Continue) return res;
    }

    return next_.geometry_end(&meta_);
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

SEXP handle_directed_edge(SEXP data, wk_handler_t* handler) {
  return catch_unwind([&] {
    vctr_view<uint64_t> edges = VECTOR_ELT(data, 0);
    DirectedEdgeReader reader(handler);
    return reader.read_features(edges);
  });
}

extern "C" SEXP ffi_handle_directed_edge(SEXP data, SEXP handler_xptr) {
  return wk_handler_run_xptr(&handle_directed_edge, data, handler_xptr);
}

SEXP handle_vertex(SEXP data, wk_handler_t* handler) {
  return catch_unwind([&] {
    vctr_view<uint64_t> vertexes = VECTOR_ELT(data, 0);
    VertexReader reader(handler);
    return reader.read_features(vertexes);
  });
}

extern "C" SEXP ffi_handle_vertex(SEXP data, SEXP handler_xptr) {
  return wk_handler_run_xptr(&handle_vertex, data, handler_xptr);
}
