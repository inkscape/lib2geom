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

template <typename T>
std::vector<std::vector<unsigned> > sweep_bounds(T const & a, T const & b) {
    Events es;
    std::vector<Rect> bounds_a, bounds_b;
    for(unsigned i = 0; i < a.size(); i++) {
        bounds_a.push_back(a[i].boundsFast());
        SweepObject *obj = new SweepObject(i, true);
        es.push_back(Event(bounds_a.back().left(), obj, false));
        es.push_back(Event(bounds_a.back().right(), obj, true));
    }
    for(unsigned i = 0; i < b.size(); i++) {
        bounds_b.push_back(b[i].boundsFast());
        SweepObject *obj = new SweepObject(i, false);
        es.push_back(Event(bounds_b.back().left(), obj, false));
        es.push_back(Event(bounds_b.back().right(), obj, true));
    }
    std::sort(es.begin(), es.end());
    SweepObjects objs = sweep(es);
    
    std::vector<std::vector<unsigned> > ret(a.size(), std::vector<unsigned>());
    for(std::vector<SweepObject>::iterator ix = objs.begin(); ix != objs.end(); ix++) {
        unsigned i = ix->ix;
        ret[i].resize(ix->intersects.size());
        for(std::vector<unsigned>::iterator jx = ix->intersects.begin(); jx != ix->intersects.end(); jx++) {
            unsigned j = *jx;
            if(bounds_a[i].intersects(bounds_b[j])) ret[i].push_back(j);
        }
    }
    return ret;
}

}

#endif
