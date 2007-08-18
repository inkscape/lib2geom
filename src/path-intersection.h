#ifndef __GEOM_PATH_INTERSECTION_H
#define __GEOM_PATH_INTERSECTION_H

#include "path.h"

#include "crossing.h"

namespace Geom {

int winding(Path const &path, Point p);

inline bool contains(const Path & p, Point i, bool evenodd = false) {
    return (evenodd ? winding(p, i) % 2 : winding(p, i)) != 0;
}

Crossings crossings(const Path & a, const Path & b);
Crossings self_crossings(const Path &a);

}

#endif
