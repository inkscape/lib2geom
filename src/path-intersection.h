#ifndef __GEOM_PATH_INTERSECTION_H
#define __GEOM_PATH_INTERSECTION_H

#include "path.h"

#include "crossing.h"

namespace Geom {

int winding(Path const &path, Point p);

inline bool contains(Path const & p, Point i, bool evenodd = false) {
    return (evenodd ? winding(p, i) % 2 : winding(p, i)) != 0;
}

Crossings crossings(Path const & a, Path const & b);
Crossings self_crossings(Path const &a);
CrossingSet crossings_among(std::vector<Path> const & a);

}

#endif
