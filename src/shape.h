#ifndef __2GEOM_SHAPE_H
#define __2GEOM_SHAPE_H

#include <vector>
#include <list>
#include <set>

#include "path.h"
#include "path-intersection.h"
//TODO: BBOX optimizations

namespace Geom { 

typedef std::list<Path> Paths;

enum BoolOp { UNION, SUBTRACT, INTERSECT };

class Shape {
	Path outer;
	Paths holes;
    friend std::vector<Shape> shape_union(const Shape &, const Shape &);
    friend std::vector<Shape> shape_subtract(const Shape &, const Shape &);
    friend std::vector<Shape> shape_intersect(const Shape &, const Shape &);
    friend std::vector<Shape> path_boolean(BoolOp bo, const Path &, const Path &,
                                           CrossingsA &, CrossingsB &);
    friend Paths shapes_to_paths(const std::vector<Shape> &);
    
    Shape() {}
  public:
	Path getOuter() const { return outer; }
	Paths getHoles() const { return holes; }
	Shape(Path out, Paths in) : outer(out), holes(in) {}
};

typedef std::vector<Shape> Shapes;

inline Paths shapes_to_paths(const Shapes & s) {
    Paths ret;
    for(unsigned i = 0; i < s.size(); i++) ret.push_back(s[i].outer);
    return ret;
}

Shapes path_boolean(BoolOp bo, const Path & a, const Path & b);
Shapes path_boolean(BoolOp bo, const Path & a, const Path & b,
                               CrossingsA & cr_a, CrossingsB & cr_b);

Shapes path_subtract(const Path & a, const Path & b);
Shapes path_subtract(const Path & a, const Path & b,
                     CrossingsA & cr_a, CrossingsB & cr_b);

inline Paths path_intersect_reverse(const Path & a, const Path & b,
                            CrossingsA & cr_a, CrossingsB & cr_b );
                     
inline Shapes path_union(const Path & a, const Path & b) {
    return path_boolean(UNION, a, b);
}
inline Shapes path_union(const Path & a, const Path & b, 
                         CrossingsA & cr_a, CrossingsB & cr_b) {
    return path_boolean(UNION, a, b, cr_a, cr_b);
}

inline Shapes path_subtract_reverse(const Path & a, const Path & b) {
    return path_boolean(SUBTRACT, a, b);
}
inline Shapes path_subtract_reverse(const Path & a, const Path & b,
                                    CrossingsA & cr_a, CrossingsB & cr_b ) {
    return path_boolean(SUBTRACT, a, b, cr_a, cr_b);
}

inline Paths path_intersect(const Path & a, const Path & b) {
    return shapes_to_paths(path_boolean(INTERSECT, a, b));
}
inline Paths path_intersect(const Path & a, const Path & b,
                            CrossingsA & cr_a, CrossingsB & cr_b ) {
    return shapes_to_paths(path_boolean(INTERSECT, a, b, cr_a, cr_b));
}

}

#endif
