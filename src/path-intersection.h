#ifndef __GEOM_PATH_INTERSECTION_H
#define __GEOM_PATH_INTERSECTION_H

#include "path.h"

#include "crossing.h"

namespace Geom {

int winding(Path const &path, Point p);
bool path_direction(Path const &p);

inline bool contains(Path const & p, Point i, bool evenodd = false) {
    return (evenodd ? winding(p, i) % 2 : winding(p, i)) != 0;
}

struct Crosser { virtual Crossings operator()(Curve const &a, Curve const&b) = 0; };

struct SBCrosser : Crosser { Crossings operator()(Curve const &a, Curve const &b); };
struct BezCrosser : Crosser { Crossings operator()(Curve const &a, Curve const &b); };

Crossings crossings(Path const & a, Path const & b, Crosser &c);
Crossings self_crossings(Path const &a, Crosser &c);
CrossingSet crossings_among(std::vector<Path> const & a, Crosser &c);

inline Crossings crossings(Path const & a, Path const & b) {
    SBCrosser c = SBCrosser();
    return crossings(a, b, c);
}
inline Crossings self_crossings(Path const & a) {
    SBCrosser c = SBCrosser();
    return self_crossings(a, c);
}
inline CrossingSet crossings_among(std::vector<Path> const & a) {
    SBCrosser c = SBCrosser();
    return crossings_among(a, c);
}

}

#endif
