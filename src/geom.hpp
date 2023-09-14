#pragma once

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <vector>
#include "h3/h3api.h"

// cartesian coords
struct Coord : LatLng {
  friend bool operator==(const Coord& a, const Coord& b) {
    static const double epsilon = std::numeric_limits<double>::epsilon();
    return std::abs(a.lat - b.lat) < epsilon && std::abs(a.lng - b.lng) < epsilon;
  }

  friend bool operator!=(const Coord& a, const Coord& b) { return !(a == b); }
};

// normal vector
struct NVector {
  double x;
  double y;
  double z;

  // vector scale
  friend NVector operator*(const double scalar, const NVector& vec) { return vec.scale(scalar); }

  // vector scale
  friend NVector operator/(const NVector& vec, const double scalar) { return vec.scale(1 / scalar); }

  // vector scale
  NVector scale(const double scalar) const { return {scalar * x, scalar * y, scalar * z}; }

  // vector addition
  friend NVector operator+(const NVector& a, const NVector& b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }

  // vector difference
  friend NVector operator-(const NVector& a, const NVector& b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }

  // dot-product
  friend double operator*(const NVector& a, const NVector& b) { return a.dot(b); }

  // dot-product
  double dot(const NVector& vec) const { return x * vec.x + y * vec.y + z * vec.z; }

  // cross product
  NVector cross(const NVector& vec) const {
    // clang-format off
    return {y * vec.z - z * vec.y,
            z * vec.x - x * vec.z,
            x * vec.y - y * vec.x};
    // clang-format on
  }

  // euclidean norm
  double l2norm() const { return std::sqrt(x * x + y * y + z * z); }

  // scale to unit length
  NVector normalise() const { return *this / l2norm(); }

  // angle between vctrs
  double angle_to(const NVector& vec) const { return std::atan2(cross(vec).l2norm(), dot(vec)); }

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
  double length() const { return a.angle_to(b); }

  // spherical linear interpolation
  NVector slerp(const double frac) const { return (1 - frac) * a + frac * b; }
};

/// spherical arc string
struct ArcString {
  std::vector<NVector> coords;

  ArcString() = default;
  ArcString(const std::vector<Coord>& coords) : ArcString(coords.cbegin(), coords.cend()) {}
  ArcString(std::vector<Coord>::const_iterator first, std::vector<Coord>::const_iterator last) {
    coords.reserve(std::distance(first, last));
    std::transform(first, last, std::back_inserter(coords), NVector::from_coord);
  }

  struct Iterator {
    Iterator(std::vector<NVector>::const_iterator it) : it_(it) {}

    Arc operator*() const { return {*it_, *(it_ + 1)}; }

    // Prefix increment
    Iterator& operator++() {
      ++it_;
      return *this;
    }

    friend bool operator==(const Iterator& x, const Iterator& y) { return x.it_ == y.it_; }
    friend bool operator!=(const Iterator& x, const Iterator& y) { return !(x == y); }

  private:
    std::vector<NVector>::const_iterator it_;
  };

  Iterator begin() const { return coords.begin(); }
  Iterator end() const { return coords.end() - !coords.empty(); }
};

/// closed arc string
struct CurvedRing : ArcString {
  CurvedRing() = default;
  CurvedRing(const std::vector<Coord>& coords) : CurvedRing(coords.cbegin(), coords.cend()) {}
  CurvedRing(std::vector<Coord>::const_iterator first, std::vector<Coord>::const_iterator last) {
    const bool needs_close = *first != *(last - 1);

    // reserve enough space for closed ring
    coords.reserve(std::distance(first, last) + needs_close);
    std::transform(first, last, std::back_inserter(coords), NVector::from_coord);

    // ensure ring is closed
    if (needs_close) coords.push_back(coords.front());
  }

  // point in closed arc string
  bool contains(const NVector& coord) const {
    if (coords.size() < 3) return false;

    // spherical winding number derived from
    // http://geomalgorithms.com/a03-_inclusion.html
    int wn = 0;
    for (const auto&& [a, b] : *this) {
      if (a.cross(coord).y <= 0) {
        if (b.cross(coord).y > 0 && a.cross(b) * coord > 0) {
          ++wn;
        }
      } else if (b.cross(coord).y <= 0 && a.cross(b) * coord < 0) {
        --wn;
      }
    }

    return wn;
  }
};

/// curved polygon
struct CurvedPolygon {
  CurvedRing exterior;
  std::vector<CurvedRing> interiors;

  CurvedPolygon() = default;
  CurvedPolygon(const std::vector<Coord>& coords, const std::vector<size_t>& lengths) {
    if (coords.empty()) return;
    if (std::accumulate(lengths.begin(), lengths.end(), 0UL) != coords.size())
      throw std::invalid_argument("coords.size() must equal sum of lengths");

    interiors.reserve(lengths.size() - 1);

    // build rings
    auto it = coords.begin();
    for (auto length : lengths) {
      if (it == coords.begin())
        exterior = {it, it + length};
      else
        interiors.push_back({it, it + length});

      it += length;
    }
  }

  // point in polygon
  bool contains(NVector coord) const {
    if (!exterior.contains(coord)) return false;

    for (const auto& interior : interiors) {
      if (interior.contains(coord)) return false;
    }

    return true;
  }
};
