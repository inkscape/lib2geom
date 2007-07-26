#include "shape.h"

namespace Geom {

bool disjoint(const Path & a, const Path & b) {
    return !contains(a, b.initialPoint()) && !contains(b, a.initialPoint());
}

//Assumes that the shapes follow the invariants, and ccw outer, cw inner
Shapes unify(BoolOp btype, const Shape & a, const Shape & b) {
    //Get sorted sets of crossings
    Crossings cr = crossings(a.outer, b.outer);
    CrossingsA cr_a(cr.begin(), cr.end());
    CrossingsB cr_b(cr_a.begin(), cr_a.end());
    
    Shapes ret;
    Shape main;
    
    switch(btype) {
    case UNION:
        if(cr.empty() && disjoint(a, b)) { ret.push_back(a); ret.push_back(b); return ret; }
        ret = path_union(a.outer, b.outer, cr_a, cr_b);
        break;
    case SUBTRACT:
        if(cr.empty() && disjoint(a, b)) { ret.push_back(a); return a; }
        ret = path_subtract(a.outer, b.outer, cr_a, cr_b);
        break;
    case INTERSECT:
        if(cr.empty() && disjoint(a, b)) { return ret; }
        ret = path_boolean(INTERSECT, a.outer, b.outer, cr_a, cr_b);
        break;
    }
    
    //Copies of the holes, so that some may be removed / replaced by portions
    Paths holes[] = { a.holes, b.holes };
    
    Paths inters = btype == INTERSECT ? shapes_to_path(ret) :
                   path_intersect(a.outer, b.outer, cr_a, cr_b);
    for(Paths::iterator inner = inters.begin(); inner != inters.end(); inner++) {
        switch(btype) {
                Paths on_edge[2];
                    
                            Shapes ps = path_subtract(*k, *j, hcr_a, hcr_b);
                            for(unsigned si = 0; si < ps.size(); si++) {
                                ret.push_back();
                            }
        case UNION: {
            Paths withins[2];
            for(unsigned p = 0; p < 2; p++) {
                for ( Paths::iterator holei = holes[p].begin();
                        holei != holes[p].end(); holei++ ) {
                    Crossings hcr = crossings(*inner, *holei);
                    CrossingsA hcr_a(hcr.begin(), hcr.end());
                    CrossingsB hcr_b(hcr_a.begin(), hcr_a.end());
                    Paths innards = path_intersect(*holei, *inner, hcr_a, hcr_b);
                    if(!innards.empty()) {
                        withins[p].insert(withins[p].end(), innards.begin(), innards.end());
                    
                        //replaces the original holes entry with the remaing fragments
                        Paths remains = shapes_to_paths(path_subtract_reverse(*holei, *inner, hcr_a, hcr_b));
                        holes[p].insert(holei, remains.begin(), remains.end());
                        holes[p].erase(holei);
                    }
                }
            }
            unsigned mj = withins[0].size(), mk = withins[1].size();
            for(Paths::iterator j = withins[0].begin(); j!= withins[0].end(); j++) {
                for(Paths::iterator k = withins[1].begin(); k!= withins[1].end(); k++) {
                    Crossings hcr = crossings(*j, *k);
                    if(!hcr.empty()) {
                        CrossingsA hcr_a(hcr.begin(), hcr.end());
                        CrossingsB hcr_b(hcr_a.begin(), hcr_a.end());
                        //By the nature of intersect, we don't need to accumulate
                        Paths ps = path_intersect(*j, *k, hcr_a, hcr_b);
                        ret.holes.insert(ret.holes.end(), ps.begin(), ps.end());
                    }
                }
            }
            break;
        }
        case SUBTRACT: {
            Paths withins[2];
            for(unsigned p = 0; p < 2; p++) {
                for ( Paths::iterator holei = holes[p].begin();
                        holei != holes[p].end(); holei++ ) {
                    Crossings hcr = crossings(*inner, *holei);
                    if(hcr.empty()) {
                        if(contains(*inner, holei->initialPoint())) {
                            withins[p*2].push_back(*holei);
                        }
                    } else {
                        CrossingsA hcr_a(hcr.begin(), hcr.end());
                        CrossingsB hcr_b(hcr_a.begin(), hcr_a.end());
                        Paths ps = path_intersect(*inner, *holei, hcr_a, hcr_b);
                        withins[p*2+1].insert(withins[p*2+1].end(), ps.start(), ps.end());
                    }
                }
            }
            for(unsigned p = 0; p < 2; p++) {
                for(unsigned add = 0; add < 2; add++) {
                    unsigned n = p + add;
                    
                }
            }
            break;
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
