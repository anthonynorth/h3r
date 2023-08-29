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
  using difference_type = ptrdiff_t;
  using size_type = ptrdiff_t;
  using pointer = value*;
  using reference = value&;

  vctr_ptr(const SEXP data, size_type idx = 0) : data_(data), idx_(idx) {
    using namespace std::literals;

    const int expected = sexp_type<T>();
    if (TYPEOF(data) != expected)
      throw std::invalid_argument("Expected "s + Rf_type2char(expected) + " vector"s);
  }

  SEXP data() const { return data_; }
  operator SEXP() const { return data_; }

  value_type operator*() { return {*this, idx_}; }
  value_type operator*() const { return {*this, idx_}; }

  value_type operator[](size_type i) { return {*this, i}; }
  value_type operator[](size_type i) const { return {*this, i}; }

  vctr_ptr& operator++() {
    ++idx_;
    return *this;
  };

  vctr_ptr& operator--() {
    --idx_;
    return *this;
  };

  vctr_ptr& operator+=(difference_type n) {
    idx_ += n;
    return *this;
  }

  vctr_ptr& operator-=(difference_type n) {
    idx_ -= n;
    return *this;
  }

  friend bool operator==(const vctr_ptr& x, const vctr_ptr& y) { return x.idx_ == y.idx_; }
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
  friend difference_type operator-(const vctr_ptr& x, const vctr_ptr& y) { return x.idx_ - y.idx_; }

  // wrapper for deref-assignment
  struct value {
    const vctr_ptr& ptr;
    const size_type idx;

    T operator*() const { return ptr.get(idx); }
    operator T() const { return ptr.get(idx); }
    void operator=(T value) { return ptr.set(idx, value); }
    void operator=(value value) { return ptr.set(idx, value); }
  };

private:
  SEXP data_;
  size_type idx_;

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
      if (SEXP str = STRING_ELT(data_, i); str == NA_STRING)
        return std::string_view();
      else
        return std::string_view(CHAR(str));
    else if constexpr (std::is_same_v<SEXP, t>)
      return VECTOR_ELT(data_, i);
    else
      static_assert(unsupported<T>, "Unsupported type");
  }

  // set underyling value
  void set(size_type i, T value) const {
    static_assert(!is_const, "Unsupported for const");
    using t = std::remove_cv_t<T>;

    if constexpr (std::is_same_v<int, t>)
      return SET_INTEGER_ELT(data_, i, value);
    else if constexpr (std::is_same_v<int64_t, t> || std::is_same_v<uint64_t, t>)
      return SET_REAL_ELT(data_, i, bit_cast<double>(value));
    else if constexpr (std::is_same_v<double, t>)
      return SET_REAL_ELT(data_, i, value);
    else if constexpr (std::is_same_v<std::string_view, t>)
      if (value.data() == nullptr)
        return SET_STRING_ELT(data_, i, NA_STRING);
      else
        return SET_STRING_ELT(data_, i, Rf_mkCharLenCE(value.data(), value.size(), CE_BYTES));
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

  vctr() : vctr(0) {}
  explicit vctr(size_type size) : ptr_(protect(allocate(size))), size_(size), capacity_(size) {}
  ~vctr() { unprotect(ptr_); }

  value_type operator[](size_type i) { return ptr_[i]; }

  operator SEXP() const {
    if (size() != capacity()) return allocate_and_copy(size());

    return ptr_.data();
  }

  size_type size() const { return size_; }
  size_type capacity() const { return capacity_; }

  void reserve(size_type n) {
    if (n <= capacity()) return;

    capacity_ = n;
    ptr_ = reprotect(allocate_and_copy(n));
  }

  void push_back(T value) {
    if (size() >= capacity()) reserve(size() == 0 ? 1 : size() * 2);
    ptr_[size_++] = value;
  }

  iterator begin() { return ptr_; }
  iterator end() { return ptr_ + size(); }

private:
  int pidx_ = -1;
  pointer ptr_;
  size_type size_;
  size_type capacity_;

  pointer allocate(size_type n) const { return Rf_allocVector(sexp_type<T>(), n); }

  pointer allocate_and_copy(size_type n) const {
    pointer tmp = PROTECT(allocate(n));
    std::copy(ptr_, ptr_ + std::min(size(), n), tmp);
    Rf_setAttrib(tmp.data(), R_ClassSymbol, Rf_getAttrib(ptr_.data(), R_ClassSymbol));
    UNPROTECT(1);
    return tmp;
  }

  pointer protect(pointer ptr) {
    PROTECT_WITH_INDEX(ptr, &pidx_);
    return ptr;
  }

  pointer reprotect(pointer ptr) {
    REPROTECT(ptr, pidx_);
    return ptr;
  }

  void unprotect(pointer ptr) { UNPROTECT(1); }
};
