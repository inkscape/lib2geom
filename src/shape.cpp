#include "shape.h"

namespace Geom 



//Assumes that the shapes follow the invariants, and ccw outer, cw inner
Shape unify(const Shape & a, const Shape & b) {
    //Get sorted sets of crossings
    Crossings cr = crossings(a.outer, b.outer);
    CrossingsA cr_a(cr.begin(), cr.end());
    CrossingsB cr_b(cr_a.begin(), cr_a.end());
    
    Shape ret = path_union(a.outer, b.outer, cr_a, cr_b).front();
    
    //Copies of the holes, so that some may be removed
    Paths holes[] = { a.holes, b.holes };
    
    //Stores groups of possibly intersecting holes
    std::vector<std::pair<Paths, Paths> > inner_holes;
    
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
                Paths remains = shapes_to_paths(path_subtract_reverse(*holei, *inner, hcr_a, hcr_b));
                holes[p].insert(holei, remains.begin(), remains.end());
                holes[p].erase(holei);
            }
        }
        inner_holes.push_back(std::pair<Paths, Paths>(withins[0], withins[1]));
    }
    
    //intersect the holes
    for(unsigned i = 0; i < inner_holes.size(); i++) {
        unsigned mj = inner_holes[i].first.size(), mk = inner_holes[i].second.size();
        for(Paths::iterator j = inner_holes[i].first.begin();
                            j!= inner_holes[i].first.end(); j++) {
            for(Paths::iterator k = inner_holes[i].second.begin();
                                k!= inner_holes[i].second.end(); k++) {
                Crossings hcr = crossings(*j, *k);
                if(!hcr.empty()) {
                    CrossingsA hcr_a(hcr.begin(), hcr.end());
                    CrossingsB hcr_b(hcr_a.begin(), hcr_a.end());
                    
                    Paths ps = path_intersect(*j, *k, hcr_a, hcr_b);
                    ret.holes.insert(ret.holes.end(), ps.begin(), ps.end());
                }
            }
        }
    }
    for(unsigned p = 0; p < 2; p++)
        ret.holes.insert(ret.holes.end(), holes[p].begin(), holes[p].end());
    return ret;
}

bool logical_xor (bool a, bool b) { return (a || b) && !(a && b); }

/*This just reverses b and calls path_boolean(SUBTRACT, ...).
  It turns out path_subtract_reverse is used much more often anyway. */
Shapes path_subtract(const Path & a, const Path & b,
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

/* This handles all the boolean ops in one function.  The middle function
 * is the main area of similarity between each.  The initial and ending
 * code is custom for each. */
Shapes path_boolean(BoolOp btype,
                    const Path & a, const Path & b,
                    CrossingsA & cr_a, CrossingsB & cr_b) {
    Shapes ret;
    //Handle the cases where there are no actual boundary intersections:
    if(cr_a.empty()) {
        Shape s;
        switch(btype) {
        case UNION:
            //For inclusion, return the outside path
            if(contains(a, b.initialPoint())) {      s.outer = a; ret.push_back(s); }
            else if(contains(b, a.initialPoint())) { s.outer = b; ret.push_back(s); }
            //For disjoint, return both
            else {                  s.outer = a; ret.push_back(s);
                                    s.outer = b; ret.push_back(s); }
            return ret;
        case SUBTRACT:
            //Return an empty list when b includes a
            if(contains(b, a.initialPoint())) { return ret; } else
            //Return a shape with an outer formed by a, and a hole formed by b
            if(contains(a, b.initialPoint())) {
                s.outer = a;
                s.holes.push_back(b);
                ret.push_back(s);
            //Otherwise, they are disjoint, just return a
            } else {
                s.outer = a;
                ret.push_back(s);
            }
            return ret;
        case INTERSECT:
            //For inclusion, return the inside path
            if(contains(b, a.initialPoint())) {      s.outer = a; ret.push_back(s); }
            else if(contains(a, b.initialPoint())) { s.outer = b; ret.push_back(s); }
            //For disjoint, none
            return ret;
        }
    }
    
    //Process the crossings into path chunks:
    
    std::vector<Path> chunks;
    for(CrossIterator it = cr_a.begin(); it == cr_a.end(); it++) {
        Path res;
        CrossIterator i = it;
        do {
            //next is after oit
            CrossIterator oit, next;
            
            /* This logical_xor basically switches the behavior of the if when
             * doing an intersection */
            if(logical_xor(it -> dir, btype == INTERSECT)) {
                oit = cr_a.lower_bound(*it);
                next = oit; next++;
                if(next == cr_a.end()) next = cr_a.begin();
                res.concat(portion(a, it->ta, next->ta));
            } else {
                oit = cr_b.lower_bound(*it);
                next = oit; next++;
                if(next == cr_b.end()) next = cr_b.begin();
                res.concat(portion(b, it->tb, next->tb));
            }
            
            //Remove all but the first crossing
            //This way the function doesn't return duplicate paths
            if (i != it) {
                cr_a.erase(*oit);
                cr_b.erase(*it);
            }
            it = next;
        } while (!(*it == *i));
        chunks.push_back(res);
    }
    
    //Process the chunks into shapes output
    
    if(chunks.empty()) { return ret; }
    
    //If we are doing a union, the result may have multiple holes
    if(btype = UNION) {
        //First, find the outer path index
        unsigned ix;
        if(chunks.size() == 1 || contains(chunks[1], chunks[0].initialPoint())) {
            ix = 0;
        } else {
           /* This should work since we've already shown that chunks[0] is
            * not the outer_path, so can be used as an exemplar inner. */
            for(unsigned i = 0; i < chunks.size(); i++) {
                if(contains(chunks[0], chunks[i].initialPoint())) {
                    ix = i;
                    break;
                }
            }
        }
        //Now we may construct the shape
        Shape s;
        s.outer = chunks[ix];
        for(unsigned i = 0; i < chunks.size(); i++) {
            /* If everything is functioning properly, these should all
             * have clockwise winding. TODO: stick some assertions in here*/
            if(i != ix) s.holes.push_back(chunks[i]);
        }
        ret.push_back(s);
    } else {
        /* Intersection and non-inclusion subtraction will result in disjoint
         * shapes without holes */
        for(unsigned i = 0; i < chunks.size(); i++) {
            Shape s;
            s.outer = chunks[i];
            ret.push_back(s);
        }
    }
    return ret;
}

}

    /*Find a crossing which is certainly on the exterior
    ValueAndTime tv_a = maxX(a.outer), tv_b = maxX(b.outer);
    CrossIterator starti;
    if(tv_b < tv_a) {
        starti = cr_a.lower_bound(dummyCross(tv_a.second));
    } else {
        starti = cr_b.lower_bound(dummyCross(tv_b.second));
    }*/
