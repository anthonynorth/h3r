#pragma once

#define R_NO_REMAP
#include <Rinternals.h>
#include <stdexcept>
#include "r-safe.hpp"
#include "wk-v1.h"

namespace wk {

enum Result : int {
  Continue = WK_CONTINUE,
  Abort = WK_ABORT,
  AbortFeature = WK_ABORT_FEATURE
};

template<typename T>
const char* fmt_geometry_type(T geometry_type) {
  static_assert(std::is_same_v<T, wk_geometery_type_enum> || std::is_same_v<T, uint32_t>);
  switch (geometry_type) {
    case WK_GEOMETRY:
      return "geometry";
    case WK_POINT:
      return "point";
    case WK_LINESTRING:
      return "linestring";
    case WK_POLYGON:
      return "polygon";
    case WK_MULTIPOINT:
      return "multipoint";
    case WK_MULTILINESTRING:
      return "multilinestring";
    case WK_MULTIPOLYGON:
      return "multipolygon";
    case WK_GEOMETRYCOLLECTION:
      return "geometrycollection";
    default:
      return "unknown";
  }
}

// derived from wk/internals
struct Handler {
  virtual ~Handler() {}

  virtual Result vector_start(const wk_vector_meta_t* meta) { return Result::Continue; }
  virtual Result feature_start(const wk_vector_meta_t* meta) { return Result::Continue; }
  virtual Result null_feature() { return Result::Continue; }
  virtual Result geometry_start(const wk_meta_t* meta) { return Result::Continue; }
  virtual Result ring_start(const wk_meta_t* meta, uint32_t size) { return Result::Continue; }
  virtual Result coord(const wk_meta_t* meta, const double* coord) { return Result::Continue; }
  virtual Result ring_end(const wk_meta_t* meta, uint32_t size) { return Result::Continue; }
  virtual Result geometry_end(const wk_meta_t* meta) { return Result::Continue; }
  virtual Result feature_end(const wk_vector_meta_t* meta) { return Result::Continue; }
  virtual SEXP vector_end(const wk_vector_meta_t* meta) { return R_NilValue; }
};

// derived from wk/internals
template <class HandlerType>
struct HandlerFactory {
  static wk_handler_t* create(HandlerType* handler_data) {
    wk_handler_t* handler = wk_handler_create();
    handler->handler_data = handler_data;

    handler->vector_start = &vector_start;
    handler->vector_end = &vector_end;

    handler->feature_start = &feature_start;
    handler->null_feature = &null_feature;
    handler->feature_end = &feature_end;

    handler->geometry_start = &geometry_start;
    handler->geometry_end = &geometry_end;

    handler->ring_start = &ring_start;
    handler->ring_end = &ring_end;

    handler->coord = &coord;

    handler->finalizer = &finalizer;

    return handler;
  }

  static SEXP create_xptr(HandlerType* handler_data, SEXP tag = R_NilValue,
                          SEXP prot = R_NilValue) {
    wk_handler_t* handler = create(handler_data);
    return wk_handler_create_xptr(handler, tag, prot);
  }

private:
  inline static void finalizer(void* handler_data) noexcept {
    HandlerType* handler = static_cast<HandlerType*>(handler_data);
    if (handler != nullptr) {
      delete handler;
    }
  }

  inline static int vector_start(const wk_vector_meta_t* meta, void* handler_data) noexcept {
    HandlerType* handler = static_cast<HandlerType*>(handler_data);
    return catch_unwind([&] { return handler->vector_start(meta); });
  }

  inline static int feature_start(const wk_vector_meta_t* meta, R_xlen_t feat_id,
                                  void* handler_data) noexcept {
    HandlerType* handler = static_cast<HandlerType*>(handler_data);
    return catch_unwind([&] { return handler->feature_start(meta); });
  }

  inline static int null_feature(void* handler_data) noexcept {
    HandlerType* handler = static_cast<HandlerType*>(handler_data);
    return catch_unwind([&] { return handler->null_feature(); });
  }

  inline static int geometry_start(const wk_meta_t* meta, uint32_t part_id,
                                   void* handler_data) noexcept {
    HandlerType* handler = static_cast<HandlerType*>(handler_data);
    return catch_unwind([&] { return handler->geometry_start(meta); });
  }

  inline static int ring_start(const wk_meta_t* meta, uint32_t size, uint32_t ring_id,
                               void* handler_data) noexcept {
    HandlerType* handler = static_cast<HandlerType*>(handler_data);
    return catch_unwind([&] { return handler->ring_start(meta, size); });
  }

  inline static int coord(const wk_meta_t* meta, const double* coord, uint32_t coord_id,
                          void* handler_data) noexcept {
    HandlerType* handler = static_cast<HandlerType*>(handler_data);
    return catch_unwind([&] { return handler->coord(meta, coord); });
  }

  inline static int ring_end(const wk_meta_t* meta, uint32_t size, uint32_t ring_id,
                             void* handler_data) noexcept {
    HandlerType* handler = static_cast<HandlerType*>(handler_data);
    return catch_unwind([&] { return handler->ring_end(meta, size); });
  }

  inline static int geometry_end(const wk_meta_t* meta, uint32_t part_id,
                                 void* handler_data) noexcept {
    HandlerType* handler = static_cast<HandlerType*>(handler_data);
    return catch_unwind([&] { return handler->geometry_end(meta); });
  }

  inline static int feature_end(const wk_vector_meta_t* meta, R_xlen_t feat_id,
                                void* handler_data) noexcept {
    HandlerType* handler = static_cast<HandlerType*>(handler_data);
    return catch_unwind([&] { return handler->feature_end(meta); });
  }

  inline static SEXP vector_end(const wk_vector_meta_t* meta, void* handler_data) noexcept {
    HandlerType* handler = static_cast<HandlerType*>(handler_data);
    return catch_unwind([&] { return handler->vector_end(meta); });
  }
};

struct NextHandler final : Handler {
  NextHandler(const wk_handler_t* handler) : handler_(handler) {}

  Result vector_start(const wk_vector_meta_t* meta) override {
    feat_id_ = -1;
    int result = handler_->vector_start(meta, handler_->handler_data);
    return Result{result};
  }

  Result feature_start(const wk_vector_meta_t* meta) override {
    part_id_ = WK_PART_ID_NONE - 1;
    int result = handler_->feature_start(meta, ++feat_id_, handler_->handler_data);
    return Result{result};
  }

  Result null_feature() override {
    int result = handler_->null_feature(handler_->handler_data);
    return Result{result};
  }

  Result geometry_start(const wk_meta_t* meta) override {
    ring_id_ = -1;
    coord_id_ = -1;
    int result = handler_->geometry_start(meta, ++part_id_, handler_->handler_data);
    return Result{result};
  }

  Result ring_start(const wk_meta_t* meta, uint32_t size) override {
    int result = handler_->ring_start(meta, size, ++ring_id_, handler_->handler_data);
    return Result{result};
  }

  Result coord(const wk_meta_t* meta, const double* coord) override {
    int result = handler_->coord(meta, coord, ++coord_id_, handler_->handler_data);
    return Result{result};
  }

  Result ring_end(const wk_meta_t* meta, uint32_t size) override {
    int result = handler_->ring_end(meta, size, ring_id_, handler_->handler_data);
    return Result{result};
  }

  Result geometry_end(const wk_meta_t* meta) override {
    int result = handler_->geometry_end(meta, part_id_, handler_->handler_data);
    return Result{result};
  }

  Result feature_end(const wk_vector_meta_t* meta) override {
    int result = handler_->feature_end(meta, feat_id_, handler_->handler_data);
    return Result{result};
  }

  SEXP vector_end(const wk_vector_meta_t* meta) override {
    return handler_->vector_end(meta, handler_->handler_data);
  }

private:
  const wk_handler_t* handler_;
  // keep track of ids and forward to external handler
  R_xlen_t feat_id_ = -1;
  uint32_t part_id_ = WK_PART_ID_NONE - 1;
  uint32_t ring_id_ = -1;
  uint32_t coord_id_ = -1;
};

};  // namespace wk
