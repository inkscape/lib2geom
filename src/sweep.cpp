#include "sweep.h"

#include <algorithm>

namespace Geom {

SweepObjects sweep(Events const & es) {
    SweepObjects returns;
    
    std::vector<SweepObject*> open[2];
    for(Events::const_iterator e = es.begin(); e != es.end(); ++e) {
        unsigned ix = e->val->on_a ? 0 : 1;
        if(e->closing) {
            //since this is the closing event, remove the object from the open list
            //std::vector<SweepObject*>::iterator it = std::lower_bound(open[ix].begin(), open[ix].end(), e->val);
            //if(it != open[ix].end()) open[ix].erase(it);
            for(unsigned i = 0; i < open[ix].size(); i++) {
                if(open[ix][i] == e->val) {
                    open[ix].erase(open[ix].begin() + i);
                    std::cout << "erased!\n";
                }
            }
        } else {
            open[ix].push_back(e->val);
        }
        if(e->val->on_a) {
            if(e->closing) {
                //Done adding stuff to this object, push to results
                SweepObject *p = e->val;
                returns.push_back(*p);
                delete p;
            } else {
                //Just beginning - add all the Bs that are already open
                for(unsigned i = 0; i < open[1].size(); i++) {
                    e->val->intersects.push_back(open[1][i]->ix);
                }
            }
        } else if(!e->closing) {
            //Opening up - add to all the As that are already open
            for(unsigned i = 0; i < open[0].size(); i++) {
                open[0][i]->intersects.push_back(e->val->ix);
            }
        }
    }
    
    return returns;
}


std::vector<std::vector<unsigned> > sweep_bounds(std::vector<Rect> const & a, std::vector<Rect> const & b) {
    Events es;
    for(unsigned i = 0; i < a.size(); i++) {
        SweepObject *obj = new SweepObject(i, true);
        es.push_back(Event(a[i].left(), obj, false));
        es.push_back(Event(a[i].right(), obj, true));
    }
    for(unsigned i = 0; i < b.size(); i++) {
        SweepObject *obj = new SweepObject(i, false);
        es.push_back(Event(b[i].left(), obj, false));
        es.push_back(Event(b[i].right(), obj, true));
    }
    std::sort(es.begin(), es.end());
    SweepObjects objs = sweep(es);
    
    std::vector<std::vector<unsigned> > ret(a.size(), std::vector<unsigned>());
    for(std::vector<SweepObject>::iterator ix = objs.begin(); ix != objs.end(); ix++) {
        unsigned i = ix->ix;
        ret[i].resize(ix->intersects.size());
        for(unsigned jp = 0; jp < ix->intersects.size(); jp++) {
            unsigned j = ix->intersects[jp];
            //if(a[i][Y].intersects(b[j][Y]))
            ret[i].push_back(j);
        }
    }
    return ret;
}

//Fake cull, until the switch to the real sweep is made.
std::vector<std::vector<unsigned> > fake_cull(unsigned a, unsigned b) {
    std::vector<std::vector<unsigned> > ret;
    
    std::vector<unsigned> all;
    for(unsigned j = 0; j < b; j++)
        all.push_back(j);
    
    for(unsigned i = 0; i < a; i++)
        ret.push_back(all);
    
    return ret;
}

}
