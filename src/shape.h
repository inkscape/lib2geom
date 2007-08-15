#ifndef __2GEOM_SHAPE_H
#define __2GEOM_SHAPE_H

#include <vector>
#include <set>

#include "region.h"

//TODO: BBOX optimizations

namespace Geom {

class Shape {
    Region outer;
    Regions inners;
    //friend Shape shape_region_boolean(bool rev, Shape const & a, Region const & b);
    friend std::vector<Shape> shape_union(Shape const &, Shape const &);
    friend std::vector<Shape> do_holes(Regions const &, Regions const &);
    friend std::vector<Shape> shape_subtract(Shape const &, Shape const &);
    friend std::vector<Shape> shape_intersect(Shape const & a, Shape const & b);
    friend std::vector<Shape> shape_exclude(Shape const & a, Shape const & b);
    friend void add_holes(std::vector<Shape> &x, Regions const &h);
  public:
    Region getOuter() const { return outer; }
    Regions getInners() const { return inners; }
    
    Shape() {}
    explicit Shape(Region const & r) : outer(r) {}
    Shape(Region const & c, Regions const & h) : outer(c), inners(h) {}
    
    Shape inverse() const;
    
    Shape operator*(Matrix const &m) const;
    
    bool fill_invariants() const;  //fast
    bool cont_invariants() const;  //semi-slow
    bool cross_invariants() const; //slow
    
    bool invariants() const {
        return fill_invariants() && cont_invariants() && cross_invariants();
    }
};

typedef std::vector<Shape> Shapes;

inline Shapes shapes_from_regions(Regions const &rs) {
    Shapes res;
    for(unsigned i = 0; i < rs.size(); i++) {
        res.push_back(Shape(rs[i]));
    }
    return res;
}

}

#endif
