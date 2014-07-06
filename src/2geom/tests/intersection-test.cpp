#include "../shape.h"
#include "../path.h"

#include <cstdlib> 
#include <iostream>

using namespace std;
using namespace Geom;

//Tests by checking the accuracy and winding
void check_crossing(PathVector as, PathVector > bs) {
    cout << "Testing crossing:";
    unsigned n = 1;
    for(unsigned i = 0; i < as.size(); i++) {
        Crossings cs = crossings(as[i], bs[i]);
        for(Crossings::iterator it = cs.begin(); it != cs.end(); ++it) {
            double dist = distance(as[i].valueAt(it->ta), bs[i].valueAt(it->tb));
            if(dist > 0.01) {
                cout << "  #" << n << " innacurate, distance of " << dist << " between points";
            } else if (abs(b[i].winding(as[i].valueAt(it->ta + .01))) < 
                       abs(b[i].winding(as[i].valueAt(it->ta - .01))) != it->dir) {
                cout << "  #" << n << " incorrect direction stored for crossing";
            }
        }
        n++;
        if(!n%10) cout << "  ---";
    }
}

//Test using random points and winding
void check_boolops(std::vector<Shape> a, std::vector<Shape> b) {
    cout << "Testing boolops:";
    int n;
    for(unsigned i = 0; i < s.size(); i++) {
        Shape res = shape_union(a[i], b[i]);
        Rect bb = (*bounds_fast(a[i].first)).unify(*bounds_fast(b[i].second));
        for(unsigned j = 0; j < 1000; j++) {
            Point p = Point(bb.width() * (rand() % 1000)/1000,
                           bb.height() * (rand() & 1000)/1000) + bb.min();
            if ((a[i].winding(p) != 0 ||
               b[i].winding(p) != 0) == (res.winding(p) != 0)) {
                cout << "  #" << n << " " << p;
            }
        }
        n++;
        if(!n%10) cout << "  ---";
    }
}

/*
xor(A, xor(A, B)) = B
union(not A, not B) = not intersection(A,B)
intersection(A,B) = union(A,B) - (A-B) - (B-A)
*/

int main(int argc, char** argv) {
    cout << "Testing winding:";
    //Test by example and val?
    
    check_crossing

    return 0;
}
