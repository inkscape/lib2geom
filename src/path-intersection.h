#include "path.h"

#include <vector>
#include <list>
#include <set>

namespace Geom {

int winding(Path const &path, Point p);

inline bool contains(const Path & p, Point i, bool evenodd = false) {
    return (evenodd ? winding(p, i) % 2 : winding(p, i)) != 0;
}

struct Crossing {
    bool dir; //True: along a, a becomes outside.
    double ta, tb;  //time on a and b of crossing
    Crossing(double t_a, double t_b, bool direction) : dir(direction), ta(t_a), tb(t_b) {}
    bool operator==(const Crossing & other) const { return dir == other.dir && ta == other.ta && tb == other.tb; }
};

struct OrderA { bool operator()(Crossing a, Crossing b) { return a.ta < b. ta; } };
struct OrderB { bool operator()(Crossing a, Crossing b) { return a.tb < b. tb; } };

typedef std::list<Crossing> Crossings;
typedef std::set<Crossing, OrderA> CrossingsA;
typedef std::set<Crossing, OrderB> CrossingsB;
typedef CrossingsA::iterator CrossIterator;

Crossings crossings(const Path & a, const Path & b);

}
