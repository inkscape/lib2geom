#ifndef __2GEOM_SWEEP_H__
#define __2GEOM_SWEEP_H__

#include <vector>
#include "d2.h"

namespace Geom {

struct SweepObject {
    unsigned ix;
    bool on_a;
    std::vector<unsigned> intersects;
    
    SweepObject(unsigned i, bool a) : ix(i), on_a(a) {}
};

typedef std::vector<SweepObject> SweepObjects;

struct Event {
    double x;
    SweepObject *val;
    bool closing;
    
    friend std::vector<SweepObject> region_pairs(std::vector<Event> const & es);
    
    Event(double t, SweepObject *v, bool c) : x(t), val(v), closing(c) {}
    
    bool operator<(Event const &other) const {
        if(x < other.x) return true;
        if(x > other.x) return false;
        return closing < other.closing;
    }
};

typedef std::vector<Event> Events;

SweepObjects sweep(Events const & es);

std::vector<std::vector<unsigned> > fake_cull(unsigned a, unsigned b);

std::vector<std::vector<unsigned> > sweep_bounds(std::vector<Rect> const & a, std::vector<Rect> const & b);

}

#endif
