#include "Shape.h"

namespace Geom {

//Assumes that the shapes follow the invariants, and ccw outer, cw inner
Shape unify(const Shape & a, const Shape & b) {
    Shape ret;
    
    //Get sorted sets of crossings
    Crossings cr = crossings(a.outer, b.outer);
    CrossingsA cr_a(cr.begin(), cr.end());
    CrossingsB cr_b(cr_a.begin(), cr_a.end());
    
    ret.outer = path_union(a.outer, b.outer, cr_a, cr_b);
    
    //Copies of the holes, so that some may be removed
    Paths holes[] = { a.holes, b.holes };
    
    //The following code 
    
    //Stores groups of possibly intersecting holes holes
    std::vector<Paths[] > inner_holes;
    
    Paths inters = path_intersect(a.outer, b.outer, cr_a, cr_b);
    for(Paths::iterator inner = inters.begin(); inner != inters.end(); inner++) {
        Paths withins[2];
        for(unsigned p = 0; p < 2; p++) {
            for ( Paths::iterator holei = holes[p].begin();
                    holei != holes[p].end(); holei++ ) {
                Crossings hcr = crossings(*inner, *holei);
                CrossingsA hcr_a(hcr.begin(), hcr.end());
                CrossingsB hcr_b(hcr_a.begin(), hcr_a.end());
                
                Paths innards = path_intersect(*holei, *inner, hcr_a, hcr_b);
                withins[p].insert(withins[p].end(), innards.begin(), innards.end());
                
                //replaces the original holes entry with the remaing fragments
                Paths remains = path_subtract_reverse(*holei, *inner, hcr_a, hcr_b);
                holes[p].insert(holei, remains.begin(), remains.end());
                holes[p].erase(holei);
            }
        }
        inner_holes.push_back(withins);
    }
    //union the holes
    for(unsigned i = 0; i < inner_holes.size(); i++) {
        for(unsigned j = 0; j < inner_holes; j++) {
            //inner_holes[i][0];
        }
    }
    return ret;
}

//The following path_* functions only work on simple paths with the same winding

Path path_union(const Path & a, const Path & b,
                          CrossingsA & cr_a, CrossingsB & cr_b ) {
    Path ret;
    if(cr_a.empty()) {
        if(inside(b, a)) { return a; } else
        if(inside(a, b)) { return b; }
        return ret; //TODO: technically this should return 2.. somehow..
    }
    //builds the outer path, by switching an iterator between a and b as necessary, and appending chunks
    CrossIterator it = cr_a.begin();
    do {
        CrossIterator next;
        
        //traverse the outside path
        if(it -> dir) {
            //A is outside, so take a chunk from it
            next = cr_a.upper_bound(*it);
            if(next == cr_a.end()) next = cr_a.begin();
            ret.concat(portion(a, it->ta, next->ta));
        } else {
            //B is outside, so take a chunk from it
            next = cr_b.upper_bound(*it);
            if(next == cr_b.end()) next = cr_b.begin();
            ret.concat(portion(a, it->tb, next->tb));
        }
        
        it = next;
    } while (!(*it == *cr_a.begin()));
    
    return ret;
}

/* This function works as follows:
 * 1) Select a crossing (it)
 * 2) Traverse the inner path, at each crossing removing it from the list,
 *      such that it will not be selected in step 1.
 * 3) Repeat until there are no untouched crossings.
 */
Paths path_intersect(const Path & a, const Path & b,
                     CrossingsA & cr_a, CrossingsB & cr_b ) {
    Paths ret;
    if(cr_a.empty()) {
        if(inside(a, b)) { ret.push_back(a); } else
        if(inside(b, a)) { ret.push_back(b); }
        return ret;
    }
    for(CrossIterator it = cr_a.begin(); it == cr_a.end(); it++) {
        /*The following code resembles path_union except that
          it removes things as it hits them, and reverses the preference
          for branches */
        Path res;
        CrossIterator i = it;
        do {
            //next is after oit
            CrossIterator oit, next;
            
            //traverse the inside path
            if(it -> dir) {
                //A is outside: B is inside, so take a chunk from it
                oit = cr_b.lower_bound(*it);
                next = oit; next++;
                if(next == cr_b.end()) next = cr_b.begin();
                res.concat(portion(b, it->tb, next->tb));
            } else {
                //Otherwise, A is inside, so take a chunk from it
                oit = cr_a.lower_bound(*it);
                next = oit; next++;
                if(next == cr_a.end()) next = cr_a.begin();
                ret.concat(portion(a, it->ta, next->ta));
            }
            
            //Remove all but the first crossing
            //This way the function doesn't return duplicate paths
            if (i != it) {
                cr_a.erase(oit);
                cr_b.erase(it);
            }
            it = next;
        } while (!(*i == *it));
    }
    
    return ret;
}

/*This just reverses b and calls path_subtract_reverse.
  It turns out path_subtract_reverse is used much more often anyway,
  due to the natural hole winding */
Paths path_subtract(const Path & a, const Path & b,
                    CrossingsA & cr_a, CrossingsB & cr_b ) {
    CrossingsB new_cr_b;
    double max = b.size();
    for(CrossIterator it = cr_b.begin(); it != cr_b.end(); it++) {
        Crossing x = *it;
        x.tb = max - x.tb;
        new_cr_b.insert(x);
    }
    return path_subtract_reverse(a, b.reverse(), cr_a, new_cr_b);
}

//a must be ccw, b cw.
/* This code resembles both path_intersect and path_union.  In order
 * to subtract a reversed path from a regular, really the same logic
 * at each crossing is used as in union.  However, as one path is
 * reversed (and by the very nature of subtraction), there may be
 * multiple outputs, path_intersection's contribution.
 */
Paths path_subtract_reverse(const Path & a, const Path & b,
                            CrossingsA & cr_a, CrossingsB & cr_b) {
    Paths ret;
    if(cr_a.empty()) {
        //If b is within, or disjoint from a, return a
        if(!inside(a, b)) ret.push_back(a);
        return ret;
    }
    for(CrossIterator it = cr_a.begin(); it == cr_a.end(); it++) {
        /*The following code resembles path_union except that
          it removes things as it hits them, and reverses the preference
          for branches */
        Path res;
        CrossIterator i = it;
        do {
            //next is after oit
            CrossIterator oit, next;
            
            /* Since b is reversed, the inside/outside state of the two
               paths at every crossings are the same. */
            if(it -> dir) {
                //Paths are outside, so pick A
                oit = cr_a.lower_bound(*it);
                next = oit; next++;
                if(next == cr_a.end()) next = cr_a.begin();
                ret.concat(portion(a, it->ta, next->ta));
            } else {
                //Paths are inside (intersecting), pick B - it's the shape carved out of A.
                oit = cr_b.lower_bound(*it);
                next = oit; next++;
                if(next == cr_b.end()) next = cr_b.begin();
                ret.concat(portion(b, it->tb, next->tb));
            }
            
            //Remove all but the first crossing
            //This way the function doesn't return duplicate paths
            if (i != it) {
                cr_a.erase(*oit);
                cr_b.erase(*it);
            }
            it = next;
        } while (!(*it == *i));
    }
    
    return ret;
}


    /*Find a crossing which is certainly on the exterior
    ValueAndTime tv_a = maxX(a.outer), tv_b = maxX(b.outer);
    CrossIterator starti;
    if(tv_b < tv_a) {
        starti = cr_a.lower_bound(dummyCross(tv_a.second));
    } else {
        starti = cr_b.lower_bound(dummyCross(tv_b.second));
    }*/
}
