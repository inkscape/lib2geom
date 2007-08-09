#ifndef __2GEOM_SHAPE_H
#define __2GEOM_SHAPE_H

#include <vector>
#include <set>

#include "path.h"
#include "path-intersection.h"
//TODO: BBOX optimizations

namespace Geom { 

typedef std::vector<Path> Paths;

enum BoolOp { UNION, SUBTRACT, INTERSECT };

//Not yet used
struct Portion {
    Path *source;
    Interval i;
    
    double from() const { return i.min(); }
    double to() const { return i.max(); }
};
class Region {
    virtual Path getBoundary() const;
    virtual Rect boundsFast() const;
public:
    virtual ~Region() {}
};
class PathRegion : public Region {
    Path boundary;
    boost::optional<Rect> box;
public:
    PathRegion(Path const &p) : boundary(p) {}
    PathRegion(Path const &p, boost::optional<Rect> const &b) : boundary(p), box(b) {}
    
    Path getBoundary() const { return boundary; }
    Rect boundsFast() {
        if(!box) box = boost::optional<Rect>(boundary.boundsFast());
        return *box;
    }
};
class PortionRegion : public Region {
    std::vector<Portion> portions;
    boost::optional<Rect> box;
public:
    PortionRegion(std::vector<Portion> ps) : portions(ps) {}
    
    Path getBoundary() const {
        Path ret;
        for(unsigned i = 0; i < portions.size(); i++) {
            const Portion p = portions[i];
            p.source->appendPortionTo(ret, p.from(), p.to());
        }
        return ret;
    }
    operator const PathRegion() { return PathRegion(getBoundary(), box); }
    Rect boundsFast() {
        if(!box) {
/*            const Portion p = portions.front();
              box = p.source->boundsLocal(p.i));
              for(unsigned i = 0; i < portions.size(); i++) {
              p = portions[i];
              box.unionWith(p.source->boundsLocal(p.i));
              }*/
            box = boost::optional<Rect>(getBoundary().boundsFast());
        }
        return *box;
    }
};

class Shape {
    Path outer;
    Paths holes;
    friend std::vector<Shape> shape_union(Shape const &, Shape const &);
    friend std::vector<Shape> shape_subtract(Shape const &, Shape const &);
    friend std::vector<Shape> shape_intersect(Shape const &, Shape const &);
    friend std::vector<Shape> path_boolean(BoolOp, Path const &, Path const &,
                                           Crossings &, Crossings &);
    friend void add_holes(std::vector<Shape> &x, Paths const &h);
    
public:
    Path getOuter() const { return outer; }
    Paths getHoles() const { return holes; }
	
    Shape() {}
    explicit Shape(Path out) : outer(out) {}
    Shape(Path out, Paths in) : outer(out), holes(in) {}
	
    bool isEmpty() const { return outer.size() == 0 && holes.size() == 0; }
};

typedef std::vector<Shape> Shapes;

//assumes paths satisfy outer invariants
template<typename T>
inline Paths shapes_to_paths(T const & s) {
    Paths ret;
    for(typename T::const_iterator i = s.begin(); i != s.end(); i++) ret.push_back(i->getOuter());
    return ret;
}

template<typename T>
inline Shapes paths_to_shapes(T const & p) {
    Shapes ret;
    for(typename T::const_iterator i = p.begin(); i != p.end(); i++) ret.push_back(Shape(*i));
    return ret;
} 

void add_holes(Shapes &x, Paths const &h);

unsigned outer_index(std::vector<Path> const &ps);

//These are the various helper functions for path intersection.
//They are all just convenience wrappers to the main path_boolean.

Shapes path_boolean(BoolOp bo, Path const & a, Path const & b, Crossings const &cr);
inline Shapes path_boolean(BoolOp bo, Path const & a, Path const & b) {
    return path_boolean(bo, a, b, crossings(a, b));
}
Shapes path_boolean(BoolOp bo, Path const & a, Path const & b,
                    Crossings & cr_a, Crossings & cr_b);
Shapes path_boolean_reverse(BoolOp bo, Path const & a, Path const & b, Crossings const &cr);

// wrappers which encode the boolops enum in the call

inline Shapes path_subtract_reverse(Path const & a, Path const & b) { return path_boolean(SUBTRACT, a, b); }
inline Shapes path_subtract_reverse(Path const & a, Path const & b,
                                    Crossings const &cr) { return path_boolean(SUBTRACT, a, b, cr); }
inline Shapes path_subtract_reverse(Path const & a, Path const & b,
                                    Crossings & cr_a, Crossings & cr_b) { return path_boolean(SUBTRACT, a, b, cr_a, cr_b); }

inline Shapes path_union(Path const & a, Path const & b) { return path_boolean(UNION, a, b); }
inline Shapes path_union(Path const & a, Path const & b,
                         Crossings const &cr) { return path_boolean(UNION, a, b, cr); }
inline Shapes path_union(Path const & a, Path const & b, 
                         Crossings & cr_a, Crossings & cr_b) { return path_boolean(UNION, a, b, cr_a, cr_b); }

inline Paths path_intersect(Path const & a, Path const & b) {
    return shapes_to_paths<Shapes>(path_boolean(INTERSECT, a, b));
}
inline Paths path_intersect(Path const & a, Path const & b, Crossings const &cr) {
    return shapes_to_paths<Shapes>(path_boolean(INTERSECT, a, b, cr));
}
inline Paths path_intersect(Path const & a, Path const & b, Crossings & cr_a, Crossings & cr_b ) {
    return shapes_to_paths<Shapes>(path_boolean(INTERSECT, a, b, cr_a, cr_b));
}

// reversals:
 
inline Shapes path_subtract(Path const & a, Path const & b)      { return path_subtract_reverse(a, b.reverse()); }
inline Shapes path_subtract(Path const & a, Path const & b,
                            Crossings const &cr)       { return path_boolean_reverse(SUBTRACT, a, b, cr); }
                                      
inline Shapes path_union_reverse(Path const & a, Path const & b) { return path_union(a, b.reverse()); }
inline Shapes path_union_reverse(Path const & a, Path const & b,
                                 Crossings const & cr)      { return path_boolean_reverse(UNION, a, b, cr); }

inline Paths path_intersect_reverse(Path const & a, Path const & b)     { return path_intersect(a, b.reverse()); }
inline Paths path_intersect_reverse(Path const & a, Path const & b, Crossings const &cr) {
    return shapes_to_paths<Shapes>(path_boolean_reverse(INTERSECT, a, b, cr));
}

}

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
