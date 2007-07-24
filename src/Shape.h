#include <vector>
#include <list>
#include <set>
#include <utility>
#include "path.h"

//TODO: BBOX optimizations

namespace Geom { 

typedef std::list<Path> Paths;

class Shape {
	Path outer;
	Paths holes;
  public:
	Path getOuter() { return outer; }
	Paths getHoles() { return holes; }
	
	friend Shape unify(const Shape & a, const Shape & b);
};

struct Crossing {
    bool dir; //True: along a, a becomes outside.
    double ta, tb;  //time on a and b of crossing
    Crossing(double t_a, double t_b, bool direction) : ta(t_a), tb(t_b), dir(direction) {}
    bool operator==(const Crossing & other) const { return dir == other.dir && ta == other.ta && tb == other.tb; }
};

Crossing dummyCross(double t) { return Crossing(t, t, true); }

struct OrderA { bool operator()(Crossing a, Crossing b) { return a.ta < b. ta; } };
struct OrderB { bool operator()(Crossing a, Crossing b) { return a.tb < b. tb; } };

typedef std::list<Crossing> Crossings;
typedef std::set<Crossing, OrderA> CrossingsA;
typedef std::set<Crossing, OrderB> CrossingsB;
typedef CrossingsA::iterator CrossIterator;


Crossings crossings(const Path & a, const Path & b);
bool inside(const Path & inner, const Path & outer);
Path portion(const Path & p, double from, double to);

Shape unify(const Shape & a, const Shape & b);
Path path_union(const Path & a, const Path & b, CrossingsA & cr_a, CrossingsB & cr_b );
Paths path_subtract(const Path & a, const Path & b, CrossingsA & cr_a, CrossingsB & cr_b );
Paths path_subtract_reverse(const Path & a, const Path & b, CrossingsA & cr_a, CrossingsB & cr_b );
Paths path_intersect(const Path & a, const Path & b, CrossingsA & cr_a, CrossingsB & cr_b );

}
