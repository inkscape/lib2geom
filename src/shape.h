#ifndef __2GEOM_SHAPE_H
#define __2GEOM_SHAPE_H

#include <vector>
#include <set>

#include "region.h"

//TODO: BBOX optimizations

namespace Geom {

class Shape {
    Regions content;
    bool fill;
    //friend Shape shape_region_boolean(bool rev, Shape const & a, Region const & b);
    friend CrossingSet crossings_between(Shape const &a, Shape const &b);
    friend Shape shape_boolean(bool rev, Shape const &, Shape const &, CrossingSet const &);
  public:
    Shape() {}
    explicit Shape(Region const & r) {
        content = Regions(1, r);
        fill = r.fill;
    }
    explicit Shape(Regions const & r) : content(r) {
        fill = r[outer_index(r)].fill;
    }
    Shape(Regions const & r, bool f) : content(r), fill(f) {}
    
    Regions getContent() const { return content; }
    bool isFill() const { return fill; }

    Shape inverse() const;
    Shape operator*(Matrix const &m) const;
    
    bool contains(Point const &p) const;
    
    bool inside_invariants() const;  //semi-slow & easy to violate : checks that the insides are inside, the outsides are outside
    bool region_invariants() const; //semi-slow                    : checks for self crossing
    bool cross_invariants() const; //slow                          : checks that everything is disjoint
    bool invariants() const;      //vera slow (combo rombo, checks the above)
};

CrossingSet crossings_between(Shape const &a, Shape const &b);

Shape shape_boolean(bool rev, Shape const & a, Shape const & b);
inline Shape shape_union(Shape const &a, Shape const &b) { return shape_boolean(false, a, b); }
inline Shape shape_intersect(Shape const &a, Shape const &b) { return shape_boolean(true, a, b); }
inline Shape shape_subtract(Shape const &a, Shape const &b) { return shape_boolean(true, a, b.inverse()); }

Shape sanitize_paths(std::vector<Path> ps);

}

#endif
