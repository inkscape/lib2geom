#include "sweep.h"

#include <algorithm>

namespace Geom {

std::vector<std::vector<unsigned> > sweep_bounds(std::vector<Rect> a, std::vector<Rect> b) {
    std::vector<std::vector<unsigned> > pairs(a.size());
    if(a.empty() || b.empty()) return pairs;
    std::vector<Event> events[2];
    events[0].reserve(a.size()*2);
    events[1].reserve(b.size()*2);
    
    for(unsigned n = 0; n < 2; n++) {
        for(unsigned i = 0; i < a.size(); i++) {
            events[n].push_back(Event(n ? b[i].left() : a[i].left(), i, false));
            events[n].push_back(Event(n ? b[i].right() : a[i].right(), i, true));
        }
        std::sort(events[n].begin(), events[n].end());
    }

    std::vector<unsigned> open[2];
    bool n = (events[0].back() < events[1].back()) ? 0 : 1;
    for(unsigned i[] = {0,0}; i[n] < events[n].size(); i[n]++) {
        unsigned ix = events[n][i[n]].ix;
        bool closing = events[n][i[n]].closing;
        if(closing) {
            std::vector<unsigned>::iterator iter = std::find(open[n].begin(), open[n].end(), ix);
            if(iter != open[n].end()) open[n].erase(iter);
        } else {
            if(n) {
                //add to all open a
                for(unsigned j = 0; j < open[0].size(); j++) {
                    unsigned jx = open[0][j];
                    if(a[jx][Y].intersects(b[ix][Y])) {
                        pairs[jx].push_back(ix);
                    }
                }
            } else {
                //add all open b
                for(unsigned j = 0; j < open[1].size(); j++) {
                    unsigned jx = open[1][j];
                    if(b[jx][Y].intersects(a[ix][Y])) {
                        pairs[ix].push_back(jx);
                    }
                }
            }
            open[n].push_back(ix);
        }
        n = (events[!n][i[!n]] < events[n][i[n]]) ? !n : n;
    }
    return pairs;
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
