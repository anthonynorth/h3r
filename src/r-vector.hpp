#pragma once

#define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>
#include <cstdint>
#include <string>
#include <string_view>
#include "utils.hpp"

template <typename T>
constexpr int sexp_type() {
  using t = std::remove_cv_t<T>;

  if constexpr (std::is_same_v<int, t>)
    return INTSXP;
  else if constexpr (std::is_same_v<int64_t, t> || std::is_same_v<uint64_t, t>)
    return REALSXP;
  else if constexpr (std::is_same_v<double, t>)
    return REALSXP;
  else if constexpr (std::is_same_v<std::string_view, t>)
    return STRSXP;
  else if constexpr (std::is_same_v<SEXP, t>)
    return VECSXP;
  else
    static_assert(unsupported<T>, "Unsupported type");
}

// a fancy pointer for r vectors
template <typename T, bool is_const = std::is_const_v<T>>
struct vctr_ptr {
  struct value;

  using iterator_category = std::random_access_iterator_tag;
  using value_type = value;
  using size_type = R_xlen_t;
  using difference_type = R_xlen_t;

  vctr_ptr(const SEXP data, size_type idx = 0) : data_(data), idx_(idx) {
    using namespace std::literals;

    const int expected = sexp_type<T>();
    if (TYPEOF(data) != expected)
      throw std::invalid_argument("Expected "s + Rf_type2char(expected) + " vector"s);
  }

  SEXP data() const { return data_; }

  value_type operator*() { return {*this, idx_}; }
  value_type operator*() const { return {*this, idx_}; }

  value_type operator[](size_type i) { return {*this, i}; }
  value_type operator[](size_type i) const { return {*this, i}; }

  vctr_ptr& operator++() {
    ++idx_;
    return *this;
  };

  vctr_ptr& operator++(int) {
    auto temp = *this;
    ++idx_;
    return temp;
  };

  vctr_ptr& operator--() {
    --idx_;
    return *this;
  };

  vctr_ptr& operator--(int) {
    auto temp = *this;
    --idx_;
    return temp;
  };

  vctr_ptr& operator+=(difference_type n) {
    idx_ += n;
    return *this;
  }

  vctr_ptr& operator-=(difference_type n) {
    idx_ -= n;
    return *this;
  }

  friend bool operator==(const vctr_ptr& x, const vctr_ptr& y) {
    return x.idx_ == y.idx_;
  }
  friend bool operator!=(const vctr_ptr& x, const vctr_ptr& y) { return !(x == y); }
  friend bool operator<(const vctr_ptr& x, const vctr_ptr& y) { return x.idx_ < y.idx_; }
  friend bool operator>(const vctr_ptr& x, const vctr_ptr& y) { return y < x; }
  friend bool operator<=(const vctr_ptr& x, const vctr_ptr& y) { return !(y < x); }
  friend bool operator>=(const vctr_ptr& x, const vctr_ptr& y) { return !(x < y); }

  friend vctr_ptr operator+(const vctr_ptr& x, difference_type n) {
    auto temp = x;
    temp += n;
    return temp;
  }

  friend vctr_ptr operator+(difference_type n, const vctr_ptr& x) { return x + n; }

  friend vctr_ptr operator-(const vctr_ptr& x, difference_type n) {
    auto temp = x;
    temp -= n;
    return temp;
  }

  friend vctr_ptr operator-(difference_type n, const vctr_ptr& x) { return x - n; }

  // wrapper for deref-assignment
  struct value {
    const vctr_ptr& ptr;
    const size_type idx;

    T operator*() const { return ptr.get(idx); }
    operator T() const { return ptr.get(idx); }
    void operator=(const T& value) const { return ptr.set(idx, value); }
  };

private:
  SEXP data_;
  size_type idx_ = 0;

  // get underlying value
  T get(size_type i) const {
    using t = std::remove_cv_t<T>;

    if constexpr (std::is_same_v<int, t>)
      return INTEGER_ELT(data_, i);
    else if constexpr (std::is_same_v<int64_t, t> || std::is_same_v<uint64_t, t>)
      return bit_cast<T>(REAL_ELT(data_, i));
    else if constexpr (std::is_same_v<double, t>)
      return REAL_ELT(data_, i);
    else if constexpr (std::is_same_v<std::string_view, t>)
      return std::string_view(CHAR(STRING_ELT(data_, i)));
    else if constexpr (std::is_same_v<SEXP, t>)
      return VECTOR_ELT(data_, i);
    else
      static_assert(unsupported<T>, "Unsupported type");
  }

  // set underyling value
  void set(size_type i, const T& value) const {
    static_assert(!is_const, "Unsupported for const");
    using t = std::remove_cv_t<T>;

    if constexpr (std::is_same_v<int, t>)
      return SET_INTEGER_ELT(data_, i, value);
    else if constexpr (std::is_same_v<int64_t, t> || std::is_same_v<uint64_t, t>)
      return SET_REAL_ELT(data_, i, bit_cast<double>(value));
    else if constexpr (std::is_same_v<double, t>)
      return SET_REAL_ELT(data_, i, value);
    else if constexpr (std::is_same_v<std::string_view, t>)
      return SET_STRING_ELT(data_, i,
                            Rf_mkCharLenCE(value.data(), value.size(), CE_BYTES));
    else if constexpr (std::is_same_v<SEXP, t>)
      return SET_VECTOR_ELT(data_, i, value);
    else
      static_assert(unsupported<T>, "Unsupported type");
  }
};

// non-owning vctr_view
template <typename T>
struct vctr_view {
  using pointer = vctr_ptr<T, true>;
  using size_type = typename pointer::size_type;
  using value_type = T;
  using iterator = pointer;

  vctr_view(const SEXP data) : ptr_(data), size_(Rf_xlength(data)) {}

  value_type operator[](size_type i) const { return ptr_[i]; }
  operator SEXP() const { return ptr_.data(); }
  size_type size() const { return size_; }

  iterator begin() const { return ptr_; }
  iterator end() const { return ptr_ + size(); }

private:
  const pointer ptr_;
  const size_type size_;
};

// mutable vctr with stack protect
template <typename T>
struct vctr {
  using pointer = vctr_ptr<T, false>;
  using size_type = typename pointer::size_type;
  using value_type = typename pointer::value_type;
  using iterator = pointer;

  vctr(const SEXP data) : ptr_(PROTECT(data)), size_(Rf_xlength(data)) {}
  vctr(size_type size) : vctr(Rf_allocVector(sexp_type<T>(), size)) {}
  ~vctr() { UNPROTECT(1); }

  value_type operator[](size_type i) { return ptr_[i]; }
  operator SEXP() const { return ptr_.data(); }
  size_type size() const { return size_; }

  iterator begin() { return ptr_; }
  iterator end() { return ptr_ + size(); }

private:
  const pointer ptr_;
  const size_type size_;
};