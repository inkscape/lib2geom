#include <vector>
#include <list>
#include <set>
#include <utility>
#include "path.h"

//TODO: BBOX optimizations

namespace Geom { 

typedef std::list<Path> Paths;

struct Crossing {
    bool dir; //True: along a, a becomes outside.
    double ta, tb;  //time on a and b of crossing
    Crossing(double t_a, double t_b, bool direction) : ta(t_a), tb(t_b), dir(direction) {}
    bool operator==(const Crossing & other) const { return dir == other.dir && ta == other.ta && tb == other.tb; }
};

struct OrderA { bool operator()(Crossing a, Crossing b) { return a.ta < b. ta; } };
struct OrderB { bool operator()(Crossing a, Crossing b) { return a.tb < b. tb; } };

typedef std::list<Crossing> Crossings;
typedef std::set<Crossing, OrderA> CrossingsA;
typedef std::set<Crossing, OrderB> CrossingsB;
typedef CrossingsA::iterator CrossIterator;

Crossings crossings(const Path & a, const Path & b);
bool contains(const Path & p, Point i) { return p.winding(i) != 0; }
Path portion(const Path & p, double from, double to);

//Shape unify(const Shape & a, const Shape & b);

enum BoolOp { UNION, SUBTRACT, INTERSECT };

class Shape {
	Path outer;
	Paths holes;
    friend Shape unify(const Shape &, const Shape &);
    friend std::vector<Shape> path_boolean(BoolOp bo, const Path &, const Path &,
                                           CrossingsA &, CrossingsB &);
    friend Paths shapes_to_paths(const std::vector<Shape> &);
  public:
	Path getOuter() { return outer; }
	Paths getHoles() { return holes; }
};

typedef std::vector<Shape> Shapes;

inline Paths shapes_to_paths(const Shapes & s) {
    Paths ret;
    for(unsigned i = 0; i < s.size(); i++) ret.push_back(s[i].outer);
    return ret;
}

Shapes path_boolean(BoolOp bo, const Path & a);
Shapes path_boolean(BoolOp bo, const Path & a, const Path & b,
                               CrossingsA & cr_a, CrossingsB & cr_b);

Shapes path_subtract(const Path & a, const Path & b);
Shapes path_subtract(const Path & a, const Path & b,
                     CrossingsA & cr_a, CrossingsB & cr_b);

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
    return shapes_to_paths(path_boolean(INTERSECT, a, b);
}
inline Paths path_intersect(const Path & a, const Path & b,
                            CrossingsA & cr_a, CrossingsB & cr_b ) {
    return shapes_to_paths(path_boolean(INTERSECT, a, b, cr_a, cr_b));
}

}
