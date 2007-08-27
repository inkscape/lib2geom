#ifndef __GEOM_PATH_INTERSECTION_H
#define __GEOM_PATH_INTERSECTION_H

#include "path.h"

#include "crossing.h"

#include "sweep.h"

namespace Geom {

int winding(Path const &path, Point p);
bool path_direction(Path const &p);

inline bool contains(Path const & p, Point i, bool evenodd = true) {
    return (evenodd ? winding(p, i) % 2 : winding(p, i)) != 0;
}

struct Crosser { virtual Crossings operator()(Path const &a, Path const&b) = 0; };

struct SimpleCrosser : Crosser { Crossings operator()(Path const &a, Path const &b); };
struct MonoCrosser : Crosser { Crossings operator()(Path const &a, Path const &b); };



typedef SimpleCrosser DefaultCrosser;

inline std::vector<Rect> curve_bounds(Path const &x) {
    std::vector<Rect> ret;
    for(Path::const_iterator it = x.begin(); it != x.end_closed(); ++it)
        ret.push_back(it->boundsFast());
    return ret;
}

template<typename T>
Crossings curve_sweep(Path const &a, Path const &b, T t) {
    Crossings ret;
    std::vector<Rect> bounds_a = curve_bounds(a), bounds_b = curve_bounds(b);
    std::vector<std::vector<unsigned> > ixs = sweep_bounds(bounds_a, bounds_b);
    for(unsigned i = 0; i < a.size(); i++) {
        for(std::vector<unsigned>::iterator jp = ixs[i].begin(); jp != ixs[i].end(); jp++) {
            Crossings cc = t(a[i], b[*jp]);
            //TODO: remove this loop and pass in some indicators
            for(Crossings::iterator it = cc.begin(); it != cc.end(); it++) {
                ret.push_back(Crossing(it->ta + i, it->tb + *jp, it->dir));
            }
        }
    }
    return ret;
}

Crossings self_crossings(Path const & a);

inline Crossings crossings(Path const & a, Path const & b) {
    DefaultCrosser c = DefaultCrosser();
    return c(a, b);
}


}

#endif
