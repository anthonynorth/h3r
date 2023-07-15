#pragma once

#include <stdexcept>
#include "wk-v1.h"

#include "r-safe.hpp"

enum Result {
  Continue = WK_CONTINUE,
  Abort = WK_ABORT,
  AbortFeature = WK_ABORT_FEATURE
};

// derived from wk/internals
struct Handler {
  virtual ~Handler() {}

  virtual void initialize(int* dirty) {
    if (*dirty)
      throw std::runtime_error("Can't re-use this wk_handler");

    *dirty = true;
  }

  virtual Result vector_start(const wk_vector_meta_t* meta) { return Result::Continue; }
  virtual Result feature_start(const wk_vector_meta_t* meta, R_xlen_t feat_id) {
    return Result::Continue;
  }
  virtual Result null_feature() { return Result::Continue; }
  virtual Result geometry_start(const wk_meta_t* meta, uint32_t part_id) {
    return Result::Continue;
  }
  virtual Result ring_start(const wk_meta_t* meta, uint32_t size, uint32_t ring_id) {
    return Result::Continue;
  }
  virtual Result coord(const wk_meta_t* meta, const double* coord, uint32_t coord_id) {
    return Result::Continue;
  }
  virtual Result ring_end(const wk_meta_t* meta, uint32_t size, uint32_t ring_id) {
    return Result::Continue;
  }
  virtual Result geometry_end(const wk_meta_t* meta, uint32_t part_id) { return Result::Continue; }
  virtual Result feature_end(const wk_vector_meta_t* meta, R_xlen_t feat_id) {
    return Result::Continue;
  }
  virtual SEXP vector_end(const wk_vector_meta_t* meta) { return R_NilValue; }
  virtual void deinitialize() {}
};

// derived from wk/internals
template <class HandlerType>
struct HandlerFactory {
  static wk_handler_t* create(HandlerType* handler_data) {
    wk_handler_t* handler = wk_handler_create();
    handler->handler_data = handler_data;

    handler->initialize = &initialize;
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

    // handler->error = &wk_default_handler_error;
    handler->deinitialize = &deinitialize;
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

  inline static void initialize(int* dirty, void* handler_data) noexcept {
    HandlerType* handler = static_cast<HandlerType*>(handler_data);
    return catch_unwind([&] { return handler->initialize(dirty); });
  }

  inline static int vector_start(const wk_vector_meta_t* meta, void* handler_data) noexcept {
    HandlerType* handler = static_cast<HandlerType*>(handler_data);
    return catch_unwind([&] { return handler->vector_start(meta); });
  }

  inline static int feature_start(const wk_vector_meta_t* meta, R_xlen_t feat_id,
                                  void* handler_data) noexcept {
    HandlerType* handler = static_cast<HandlerType*>(handler_data);
    return catch_unwind([&] { return handler->feature_start(meta, feat_id); });
  }

  inline static int null_feature(void* handler_data) noexcept {
    HandlerType* handler = static_cast<HandlerType*>(handler_data);
    return catch_unwind([&] { return handler->null_feature(); });
  }

  inline static int geometry_start(const wk_meta_t* meta, uint32_t partId,
                                   void* handler_data) noexcept {
    HandlerType* handler = static_cast<HandlerType*>(handler_data);
    return catch_unwind([&] { return handler->geometry_start(meta, partId); });
  }

  inline static int ring_start(const wk_meta_t* meta, uint32_t size, uint32_t ringId,
                               void* handler_data) noexcept {
    HandlerType* handler = static_cast<HandlerType*>(handler_data);
    return catch_unwind([&] { return handler->ring_start(meta, size, ringId); });
  }

  inline static int coord(const wk_meta_t* meta, const double* coord, uint32_t coord_id,
                          void* handler_data) noexcept {
    HandlerType* handler = static_cast<HandlerType*>(handler_data);
    auto lambda = [&] { return handler->coord(meta, coord, coord_id); };
    return catch_unwind(lambda);
  }

  inline static int ring_end(const wk_meta_t* meta, uint32_t size, uint32_t ringId,
                             void* handler_data) noexcept {
    HandlerType* handler = static_cast<HandlerType*>(handler_data);
    return catch_unwind([&] { return handler->ring_end(meta, size, ringId); });
  }

  inline static int geometry_end(const wk_meta_t* meta, uint32_t partId,
                                 void* handler_data) noexcept {
    HandlerType* handler = static_cast<HandlerType*>(handler_data);
    return catch_unwind([&] { return handler->geometry_end(meta, partId); });
  }

  inline static int feature_end(const wk_vector_meta_t* meta, R_xlen_t feat_id,
                                void* handler_data) noexcept {
    HandlerType* handler = static_cast<HandlerType*>(handler_data);
    return catch_unwind([&] { return handler->feature_end(meta, feat_id); });
  }

  inline static SEXP vector_end(const wk_vector_meta_t* meta, void* handler_data) noexcept {
    HandlerType* handler = static_cast<HandlerType*>(handler_data);
    return catch_unwind([&] { return handler->vector_end(meta); });
  }

  inline static void deinitialize(void* handler_data) noexcept {
    HandlerType* handler = static_cast<HandlerType*>(handler_data);
    return catch_unwind([&] { return handler->deinitialize(); });
  }
};