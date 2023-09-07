#pragma once

#include <vector>
#include "h3/h3api.h"

using Coord = LatLng;

struct LineString : std::vector<Coord> {
  LineString(std::vector<Coord>&& coords): std::vector<Coord>(coords) {}
};
