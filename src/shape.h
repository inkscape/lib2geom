#ifndef __2GEOM_SHAPE_H
#define __2GEOM_SHAPE_H

#include <vector>
#include <set>

#include "region.h"

//TODO: BBOX optimizations

namespace Geom {

enum {
  BOOLOP_JUST_A  = 1,
  BOOLOP_JUST_B  = 2,
  BOOLOP_BOTH    = 4,
  BOOLOP_NEITHER = 8
};

enum {
  SHAPE_NULL         = 0,
  SHAPE_INTERSECT    = BOOLOP_BOTH,
  SHAPE_SUBTRACT_A_B = BOOLOP_JUST_B,
  SHAPE_IDENTITY_A   = BOOLOP_JUST_A | BOOLOP_BOTH,
  SHAPE_SUBTRACT_B_A = BOOLOP_JUST_A,
  SHAPE_IDENTITY_B   = BOOLOP_JUST_B | BOOLOP_BOTH,
  SHAPE_EXCLUSION    = BOOLOP_JUST_A | BOOLOP_JUST_B,
  SHAPE_UNION        = BOOLOP_JUST_A | BOOLOP_JUST_B | BOOLOP_BOTH
};

class Shape {
    Regions content;
    mutable bool fill;
    //friend Shape shape_region_boolean(bool rev, Shape const & a, Region const & b);
    friend CrossingSet crossings_between(Shape const &a, Shape const &b);
    friend Shape shape_boolean(bool rev, Shape const &, Shape const &, CrossingSet const &);
    friend Shape shape_exclude(Shape const &a, Shape const &b);

  public:
    Shape() {}
    explicit Shape(Region const & r) {
        content = Regions(1, r);
        fill = r.fill;
    }
    explicit Shape(Regions const & r) : content(r) { update_fill(); }
    explicit Shape(bool f) : fill(f) {}
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

  private:     
    void update_fill() const {
        unsigned ix = outer_index(content);
        if(ix < content.size())
            fill = content[ix].fill;
        else if(content.size() > 0)
            fill = content.front().fill;
        else
            fill = true;
    }
};

CrossingSet crossings_between(Shape const &a, Shape const &b);

Shape shape_boolean(bool rev, Shape const &, Shape const &, CrossingSet const &);
Shape shape_boolean(bool rev, Shape const &, Shape const &);

Shape shape_boolean(unsigned flags, Shape const &, Shape const &);

inline Shape shape_union(Shape const &a, Shape const &b) { return shape_boolean(false, a, b); }
inline Shape shape_intersect(Shape const &a, Shape const &b) { return shape_boolean(true, a, b); }
inline Shape shape_subtract(Shape const &a, Shape const &b) { return shape_boolean(true, a, b.inverse()); }
Shape shape_exclude(Shape const &a, Shape const &b);

Shape sanitize_paths(std::vector<Path> ps);

}

#endif
