#include "sweep.h"

#include <algorithm>

namespace Geom {

SweepObjects sweep(Events const & es) {
    SweepObjects returns;
    
    std::vector<SweepObject*> open[2];
    for(Events::const_iterator e = es.begin(); e != es.end(); ++e) {
        unsigned ix = e->val->on_a ? 1 : 0;
        if(e->closing) {
            //since this is the closing event, remove the object from the open list
            open[ix].erase(std::find(open[ix].begin(), open[ix].end(), e->val));
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
