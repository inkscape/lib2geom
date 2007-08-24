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

Crossings self_crossings(Path const &a, Crosser &c);
CrossingSet crossings_among(std::vector<Path> const & a, Crosser &c);

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
    //TODO: pass bounds into sweeper
    std::vector<std::vector<unsigned> > ixs = fake_cull(a.size(), b.size());
    for(unsigned i = 0; i < a.size(); i++) {
        for(std::vector<unsigned>::iterator jp = ixs[i].begin(); jp != ixs[i].end(); jp++) {
            if(bounds_a[i].intersects(bounds_b[*jp])) {
                Crossings cc = t(a[i], b[*jp]);
                //TODO: remove this loop and pass in some indicators
                for(Crossings::iterator it = cc.begin(); it != cc.end(); it++) {
                    ret.push_back(Crossing(it->ta + i, it->tb + *jp, it->dir));
                }
            }
        }
    }
    return ret;
}

inline Crossings crossings(Path const & a, Path const & b) {
    DefaultCrosser c = DefaultCrosser();
    return c(a, b);
}
inline Crossings self_crossings(Path const & a) {
    DefaultCrosser c = DefaultCrosser();
    return self_crossings(a, c);
}
inline CrossingSet crossings_among(std::vector<Path> const & a) {
    DefaultCrosser c = DefaultCrosser();
    return crossings_among(a, c);
}

Path monotone(Path const &p);

}

#endif
