#include "shape.h"

#include <iostream>

namespace Geom {

template<typename It>
class circ {
    typedef typename It::value_type value_type;
    It begin;
    It end;
    It iter;
    
public:
    circ() {}
    circ(It b, It e)       : begin(b), end(e), iter(b) {}
    circ(It b, It e, It i) : begin(b), end(e), iter(i) {}
    
    circ<It> &operator++() {    
        iter++;
        if (iter == end) iter = begin;
        return *this;
    }

    circ<It> operator++(int) {
        circ<It> prev(*this);
        ++(*this);
        return(prev);
    }
    
    It inner() const { return iter; }
    void setOther(It& x) const { x = iter; }
    value_type const operator*() const { return *iter; }
    bool operator==(circ<It> const &x) const {return iter == x.iter;}
    bool operator==(It const &x) const {return iter == x;}
    bool operator!=(circ<It> const &x) const {return iter != x.iter; }
    bool operator!=(It const &x) const {return iter != x; }
};

bool disjoint(const Path & a, const Path & b) {
    return !contains(a, b.initialPoint()) && !contains(b, a.initialPoint());
}

Shapes shape_union(const Shape & a, const Shape & b) {
    //Get sorted sets of crossings
    Crossings cr = crossings(a.outer, b.outer);
    if(cr.empty() && disjoint(a.outer, b.outer)) {
        Shapes returns;
        returns.push_back(a); returns.push_back(b);
        return returns;
    }
    CrossingsA cr_a(cr.begin(), cr.end());
    CrossingsB cr_b(cr_a.begin(), cr_a.end());
    std::cout << "foo\n";
    CrossingsA cr_a_copy(cr_a);
    CrossingsB cr_b_copy(cr_b);
    Shape ret = path_union(a.outer, b.outer, cr_a_copy, cr_b_copy).front();
    std::cout << "bar\n";
    //Copies of the holes, so that some may be removed / replaced by portions
    Paths holes[] = { a.holes, b.holes };
    
    Paths inters = path_intersect(a.outer, b.outer, cr_a, cr_b);
    for(Paths::iterator inter = inters.begin(); inter != inters.end(); inter++) {
        Paths withins[2];
        for(unsigned p = 0; p < 2; p++) {
            for ( Paths::iterator holei = holes[p].begin();
                    holei != holes[p].end(); holei++ ) {
                Crossings hcr = crossings(*inter, *holei);
                CrossingsA hcr_a(hcr.begin(), hcr.end());
                CrossingsB hcr_b(hcr_a.begin(), hcr_a.end());
                Paths innards = path_intersect(*holei, *inter, hcr_a, hcr_b);
                if(!innards.empty()) {
                    withins[p].insert(withins[p].end(), innards.begin(), innards.end());
                
                    //replaces the original holes entry with the remaing fragments
                    Paths remains = shapes_to_paths(path_subtract_reverse(*holei, *inter, hcr_a, hcr_b));
                    holes[p].insert(holei, remains.begin(), remains.end());
                    holes[p].erase(holei);
                }
            }
        }
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
    }
    for(unsigned p = 0; p < 2; p++)
        ret.holes.insert(ret.holes.end(), holes[p].begin(), holes[p].end());
    Shapes returns;
    returns.push_back(ret);
    return returns;
}

Shapes shape_subtract(const Shape & a, const Shape & b) {
    Shapes ret;

    //Get sorted sets of crossings
    Crossings cr = crossings(a.outer, b.outer);
    if(cr.empty() && disjoint(a.outer, b.outer)) { ret.push_back(a); return ret; }
    CrossingsA cr_a(cr.begin(), cr.end());
    CrossingsB cr_b(cr_a.begin(), cr_a.end());

    //copies, as path booleans modify their crossing lists
    CrossingsA cr_a_copy(cr_a);
    CrossingsB cr_b_copy(cr_b);
    ret = path_subtract(a.outer, b.outer, cr_a_copy, cr_b_copy);
    
    //Copies of the holes, so that some may be removed / replaced by portions
    Paths holes[] = { a.holes, b.holes };
    
    Paths inters = path_intersect(a.outer, b.outer, cr_a, cr_b);
    for(Paths::iterator inter = inters.begin(); inter != inters.end(); inter++) {
        //Appends the holes in the subtractor that are within A to the result
        for ( Paths::iterator holei = holes[1].begin();
                holei != holes[1].end(); holei++ ) {
            Crossings hcr = crossings(*inter, *holei);
            if(hcr.empty()) {
                if(contains(*inter, holei->initialPoint())) {
                    Shape s;
                    s.outer = *holei;
                    ret.push_back(s);
                }
            } else {
                CrossingsA hcr_a(hcr.begin(), hcr.end());
                CrossingsB hcr_b(hcr_a.begin(), hcr_a.end());
                Shapes ps = path_boolean(INTERSECT, *inter, *holei, hcr_a, hcr_b);
                ret.insert(ret.end(), ps.begin(), ps.end());
            }
        }
    }
    //Subtract all of A's holes from the result shapes
    Shapes returns;
    for ( unsigned i = 0; i < ret.size(); i++) {
        for ( Paths::iterator j = holes[0].begin();
                    j != holes[0].end(); j++ ) {
            Shape s; 
            s.outer = *j;
            Shapes ps = shape_subtract(ret[i], s);
            returns.insert(returns.end(), ps.begin(), ps.end());
        }
    }
    return returns;
}

Shapes shape_intersect(const Shape & a, const Shape & b) {
    Shapes ret;

    //Get sorted sets of crossings
    Crossings cr = crossings(a.outer, b.outer);
    if(cr.empty() && disjoint(a.outer, b.outer)) { return ret; }
    CrossingsA cr_a(cr.begin(), cr.end());
    CrossingsB cr_b(cr_a.begin(), cr_a.end());
    
    ret = path_boolean(INTERSECT, a.outer, b.outer, cr_a, cr_b);
    
    //Copies of the holes, so that some may be removed / replaced by portions
    Paths holes[] = { a.holes, b.holes };
    
    Shapes returns;
    for(Shapes::iterator inter = ret.begin(); inter != ret.end(); inter++) {
        //TODO: use replacement within list
        Shapes cur;
        cur.push_back(*inter);
        for(unsigned p = 0; p < 2; p++) {
            for(Paths::iterator i = holes[p].begin(); i != holes[p].end(); i++) {
                Shape s;
                s.outer = *i;
                Shapes rep;
                for(Shapes::iterator j = cur.begin(); j != cur.end(); j++) {
                    Shapes ps = shape_subtract(*j, s);
                    rep.insert(rep.end(), ps.begin(), ps.end());
                }
                cur = rep;
            }
        } 
        returns.insert(returns.end(), cur.begin(), cur.end());
    }
    return returns;
}

Shapes path_boolean(BoolOp btype, const Path & a, const Path & b) {
    Crossings cr = crossings(a, b);
    CrossingsA cr_a(cr.begin(), cr.end());
    CrossingsB cr_b(cr_a.begin(), cr_a.end());
    return path_boolean(btype, a, b, cr_a, cr_b);
}
Shapes path_subtract(const Path & a, const Path & b) {
    /* Reversing A while keeping direction data the same would result in b - a,
       but the directions are also reversed, causing it to be a - b */
    Path new_a = a.reverse();
    Crossings cr = crossings(new_a, b);
    CrossingsA cr_a(cr.begin(), cr.end());
    CrossingsB cr_b(cr_a.begin(), cr_a.end());
    return path_subtract_reverse(new_a, b, cr_a, cr_b);
}

/*This just reverses b and the crossings and calls path_boolean(SUBTRACT, ...).
  It turns out path_subtract_reverse is used much more often anyway. */
Shapes path_subtract(const Path & a, const Path & b,
                     CrossingsA & cr_a, CrossingsB & cr_b ) {
    Crossings new_cr;
    double max = b.size();
    for(CrossIterator it = cr_a.begin(); it != cr_a.end(); it++) {
        Crossing x = *it;
        if(x.tb > max) x.tb = 1 - (x.tb - max) + max; // on the last seg - flip it about
        else x.tb = max - x.tb;
        new_cr.push_back(x);
    }
    CrossingsA new_cr_a(new_cr.begin(), new_cr.end());
    CrossingsB new_cr_b(new_cr_a.begin(), new_cr_a.end());
    return path_subtract_reverse(a, b.reverse(), new_cr_a, new_cr_b);
}

bool logical_xor (bool a, bool b) { return (a || b) && !(a && b); }

/* This handles all the boolean ops in one function.  The middle function
 * is the main area of similarity between each.  The initial and ending
 * code is custom for each. */
Shapes path_boolean(BoolOp btype,
                    const Path & a, const Path & b,
                    CrossingsA & cr_a, CrossingsB & cr_b) {
    assert(cr_a.size() == cr_b.size());
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
            if(contains(b, a.initialPoint())) { } else
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
    //Traverse the crossings, creating path chunks:
    std::vector<Path> chunks;
    for(CrossIterator it = cr_a.begin(); it != cr_a.end(); ) {
        Path res;
        CrossIterator i = it;
        //this loop collects a single continuous Path (res) which is part of the result.
        do {
            //next is after oit
            circ<CrossIterator> next;
            
            /* This logical_xor basically switches the behavior of the if when
             * doing an intersection rather than subtraction/union */
            if(logical_xor(it -> dir, btype == INTERSECT)) {
                next = circ<CrossIterator>(cr_a.begin(), cr_a.end(), cr_a.find(*it));
                if(next == cr_a.end()) goto aus; //break;  //really these should never happen...
                next++;
                a.appendPortionTo(res, it->ta, (*next).ta);
            } else {
                next = circ<CrossIterator>(cr_b.begin(), cr_b.end(), cr_b.find(*it));
                if(next == cr_b.end()) goto aus; //break;
                next++;
                std::cout << "B: " << it->tb << " to " << (*next).tb << "\n";
                b.appendPortionTo(res, it->tb, (*next).tb);
            }
            //Remove all but the first crossing, This way the function doesn't return duplicate paths
            if (*i != *it) {
                cr_a.erase(*it);
                cr_b.erase(*it);
            }
            next.setOther(it);
        } while (*it != *i);
        
        std::cout << btype << " c " << res.size() << "\n";

        chunks.push_back(res);
        /* I probably should use some sort of circular iterator for the
          following, but it would have to support circular iteration of
          different paths.. perhaps supportable by circ... */
        if(it == cr_a.end()) {
            it = cr_a.begin();
            continue;
        } else if(it == cr_b.end()) {
            it = cr_b.begin();
            continue;
        }
        //These were originally preserved in order to complete the loops, now erase.
        cr_a.erase(*it);
        cr_b.erase(*it);
        it++;
    }
    aus:

    //Process the chunks into shapes output
    
    if(chunks.empty()) { return ret; }
    
    //If we are doing a union, the result may have multiple holes
    if(btype == UNION) {
        //First, find the outer path index
        unsigned ix;
        if(chunks.size() == 1 || contains(chunks[0], chunks[1].initialPoint())) {
            ix = 0;
        } else {
           /* This should work since we've already shown that chunks[0] is
            * not the outer_path, so can be used as an exemplar inner. */
            for(unsigned i = 1; i < chunks.size(); i++) {
                if(contains(chunks[i], chunks[0].initialPoint())) {
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
