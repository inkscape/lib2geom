#include "../shape.h"
#include "../path.h"

#include <cstdlib> 
#include <iostream>

using namespace std;
using namespace Geom;

//Tests by checking the accuracy, winding, and existance
void check_crossing(std::vector<std::pair<Path, Path> > ps) {
    cout << "Testing crossing:";
    unsigned n = 1;
    for(unsigned i = 0; i < ps.size(); i++) {
        Crossings cs = crossings(ps[i].first, ps[i].second);
        for(Crossings::iterator it = cs.begin(); it != cs.end(); it++) {
            double dist = distance(ps[i].first.valueAt(it->ta), ps[i].second.valueAt(it->tb));
            if(dist > 0.0001) {
                cout << "  #" << n << " innacurate, distance of " << dist << " between points";
            } else if (abs(ps[i].second.winding(ps[i].first.valueAt(it->ta + .01))) < 
                       abs(ps[i].second.winding(ps[i].first.valueAt(it->ta - .01))) != it->dir) {
                cout << "  #" << n << " incorrect direction stored for crossing";
            }
        }
        n++;
        if(!n%10) cout << "  ---";
    }
    //TODO: check for non-found intersections
}

//Test using random points and winding
void check_boolops(std::vector<std::pair<Shape, Shape> > s) {
    cout << "Testing boolops:";
    int n;
    for(unsigned i = 0; i < s.size(); i++) {
        Shape res = unify(s[i].first, s[i].second);
        Rect bb = bounds_fast(s[i].first).unify(bounds_fast(s[i].second);
        for(unsigned j = 0; j < 1000; j++) {
            Point p = Point(bb.width() * (rand() % 1000)/1000,
                           bb.height() * (rand() & 1000)/1000) + bb.min();
            if ((s[i].first.winding(p) != 0 ||
               s[i].second.winding(p) != 0) == (res.winding(p) != 0)) {
                cout << "  #" << n << " " << p;
            }
        }
        n++;
        if(!n%10) cout << "  ---";
    }
}

int main(int argc, char** argv) {
    cout << "Testing winding:";
    //Test by example and val?

    return 0;
}
