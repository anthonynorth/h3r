#pragma once

#include <vector>
#include "h3/h3api.h"

using Coord = LatLng;

// normal vector
struct NVector {
  double x;
  double y;
  double z;

  // vector scale
  template <typename S>
  friend NVector operator*(const S s, const NVector& vec) {
    return vec.scale(s);
  }

  // vector scale
  template <typename S>
  friend NVector operator/(const NVector& vec, const S s) {
    return vec.scale(1 / s);
  }

  // vector scale
  template <typename S>
  NVector scale(const S s) const {
    return {s * x, s * y, s * z};
  }

  // vector addition
  friend NVector operator+(const NVector& a, const NVector& b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
  }

  // vector difference
  friend NVector operator-(const NVector& a, const NVector& b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
  }

  // dot-product
  friend double operator*(const NVector& a, const NVector& b) { return a.dot(b); }

  // dot-product
  double dot(const NVector& vec) const { return x * vec.x + y * vec.y + z * vec.z; }

  // cross product
  NVector cross(const NVector& vec) const {
    return {y * vec.z - z * vec.y, z * vec.x - x * vec.z, x * vec.y - y * vec.x};
  }

  // euclidean norm
  double l2norm() const { return std::sqrt(x * x + y * y + z * z); }

  // scale to unit length
  NVector normalise() const { return *this / l2norm(); }

  // angle between vctrs
  double angle(const NVector& vec) const { return std::atan2(cross(vec).l2norm(), dot(vec)); }

  // convert from cartesian
  static NVector from_coord(const Coord& coord) {
    // clang-format off
    NVector v = {std::cos(coord.lat) * std::cos(coord.lng), 
                 std::cos(coord.lat) * std::sin(coord.lng),
                 std::sin(coord.lat)};
    // clang-format on
    return v.normalise();
  }

  // convert to cartesian
  Coord to_coord() const { return {std::atan2(z, std::sqrt(x * x + y * y)), std::atan2(y, x)}; }
};

/// spherical arc
struct Arc {
  NVector a;
  NVector b;

  // great-circle length
  double length() const { return a.angle(b); }

  // linear interpolation
  NVector lerp(double frac) const { return (1 - frac) * a + frac * b; }
};

struct ArcString {
  std::vector<NVector> coords;
  
  ArcString(const std::vector<Coord>& coords) {
    this->coords.resize(coords.size());
    std::transform(coords.begin(), coords.end(), this->coords.begin(), NVector::from_coord);
  }

  struct Iterator {
    Iterator(std::vector<NVector>::const_iterator it): it_(it) {}

    Arc operator*() const { return {*it_, *(it_ + 1)}; }

    // Prefix increment
    Iterator& operator++() { ++it_; return *this; }  

    friend bool operator==(const Iterator& x, const Iterator& y) { return x.it_ == y.it_; }
    friend bool operator!=(const Iterator& x, const Iterator& y) { return !(x == y); }

  private:
    std::vector<NVector>::const_iterator it_;
  };

  Iterator begin() const { return coords.begin(); }
  Iterator end() const { return coords.end() - !coords.empty(); }
};
