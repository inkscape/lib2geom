#ifndef __GEOM_CROSSING_H
#define __GEOM_CROSSING_H

#include <vector>
#include <list>
#include <set>

namespace Geom {

struct Crossing {
    bool dir; //True: along a, a becomes outside.
    double ta, tb;  //time on a and b of crossing
    Crossing(double t_a, double t_b, bool direction) : dir(direction), ta(t_a), tb(t_b) {}
    bool operator==(const Crossing & other) const { return dir == other.dir && ta == other.ta && tb == other.tb; }
    bool operator!=(const Crossing & other) const { return !(*this == other); }
};

struct OrderA { bool operator()(Crossing a, Crossing b) { return a.ta < b. ta; } };
struct OrderB { bool operator()(Crossing a, Crossing b) { return a.tb < b. tb; } };

typedef std::list<Crossing> Crossings;
typedef std::set<Crossing, OrderA> CrossingsA;
typedef std::set<Crossing, OrderB> CrossingsB;
typedef CrossingsA::iterator CrossIterator;

}

#endif
