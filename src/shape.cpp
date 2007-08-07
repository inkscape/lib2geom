#include "shape.h"

#include <iostream>

namespace Geom {

//TODO: BBox optimization

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
    
    It inner() const           { return iter; }
    void setOther(It& x) const { x = iter; }  //TODO: get the above working - this is ugly as hell!
    value_type const operator*() const       { return *iter; }
    bool operator==(circ<It> const &x) const { return iter == x.iter;}
    bool operator==(It const &x) const       { return iter == x;}
    bool operator!=(circ<It> const &x) const { return iter != x.iter; }
    bool operator!=(It const &x) const       { return iter != x; }
};

bool disjoint(Path const & a, Path const & b) {
    return !contains(a, b.initialPoint()) && !contains(b, a.initialPoint());
}

/*
class event {
    bool opening;
    double x;
};

template<typename T, typename F>
void sweep_box(std::vector<std::pair<Rect, T> > const & a, F f) {
    
    for(unsigned i = 0; i < a.size(); i++)
        a[i]
}*/

Shapes shape_union(Shape const & a, Shape const & b) {
    //Get sorted sets of crossings
    Crossings cr = crossings(a.outer, b.outer);
    if(cr.empty() && disjoint(a.outer, b.outer)) {
        Shapes returns;
        returns.push_back(a); returns.push_back(b);
        return returns;
    }
    CrossingsA cr_a(cr.begin(), cr.end());
    CrossingsB cr_b(cr_a.begin(), cr_a.end());
    CrossingsA cr_a_copy(cr_a);
    CrossingsB cr_b_copy(cr_b);
    Shape ret = path_union(a.outer, b.outer, cr_a_copy, cr_b_copy).front();
    //Copies of the holes, so that some may be removed / replaced by portions
    Paths holes[] = { a.holes, b.holes };
    
    // iterate the intersections of the paths, and deal with the holes within
    Paths inters = path_intersect(a.outer, b.outer, cr_a, cr_b);
    for(Paths::iterator inter = inters.begin(); inter != inters.end(); inter++) {
        Paths withins[2];  //These are the portions of holes that are inside the intersection
        for(unsigned p = 0; p < 2; p++) {
            for ( Paths::iterator holei = holes[p].begin();
                    holei != holes[p].end();) {
                Crossings hcr = crossings(*inter, *holei);
                if(!hcr.empty()) {
                    CrossingsA hcr_a(hcr.begin(), hcr.end());
                    CrossingsB hcr_b(hcr_a.begin(), hcr_a.end());
                    Paths innards = path_intersect_reverse(*inter, *holei, hcr);
                    if(!innards.empty()) {
                        //stash the stuff which is inside the intersection
                        withins[p].insert(withins[p].end(), innards.begin(), innards.end());
                        
                        //replaces the original holes entry with the remaining fragments
                        Paths remains = shapes_to_paths<Shapes>(path_subtract_reverse(*inter, *holei, hcr_a, hcr_b));
                        holes[p].insert(holei, remains.begin(), remains.end());
                        
                        Paths::iterator temp = holei;
                        holei++;
                        holes[p].erase(temp);
                        continue;
                    }
                } else if(contains(*inter, holei->initialPoint())) {
                    withins[p].push_back(*holei);
                    
                    Paths::iterator temp = holei;
                    holei++;
                    holes[p].erase(temp);
                    continue; 
                }
                holei++;
            }
        }
        for(Paths::iterator j = withins[0].begin(); j!= withins[0].end(); j++) {
            for(Paths::iterator k = withins[1].begin(); k!= withins[1].end(); k++) {
                Crossings hcr = crossings(*j, *k);
                //TODO: use crosses predicate
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

void add_holes(Shapes &x, Paths const &h) {
    for(Paths::const_iterator j = h.begin(); j != h.end(); j++) {
        for(Shapes::iterator i = x.begin(); i != x.end(); i++) {
            if(contains(i->outer, j->initialPoint())) {
                i->holes.push_back(*j);
                break;
            }
        }
    }
}

Shapes shape_subtract(Shape const & ac, Shape const & b) {
    //TODO: use crosses predicate
    Crossings cr = crossings(ac.outer, b.outer);
    Shapes ret;
    bool flag_inside = false;
    if(cr.empty()) {
        if(contains(b.outer, ac.outer.initialPoint())) {
            // the subtractor contains everything - need to continue though, to evaluate the holes.
            flag_inside = false;
        } else if(contains(ac.outer, b.outer.initialPoint())) {
            // the subtractor is contained within
            flag_inside = true;
        } else {
            // disjoint
            ret.push_back(ac);
            return ret;
        }
    }
    Shape a = ac;
    
    //subtractor accumulator
    Path sub_outer = b.outer.reverse();
    Paths sub_holes;
    for(Paths::const_iterator i = b.holes.begin(); i != b.holes.end(); i++) {
        sub_holes.push_back(i->reverse());
    }
    
    //First, we deal with the outer-path - add intersecting holes in a to it 
    Paths remains;  //holes which intersected - needed later to remove from islands (holes in subtractor)
    for(Paths::iterator i = a.holes.begin(); i != a.holes.end();) {
        Crossings hcr = crossings(sub_outer, *i);
        //TODO: use crosses predicate
        if(!hcr.empty()) {
            //I'm sure this is quite reassuring to the reader, but I have no idea why the following must be subtract - union should be the one to use
            Shape new_sub = path_boolean(SUBTRACT, sub_outer, *i).front(); //path_union(sub_outer, *i, hcr).front();
            new_sub.holes.insert(new_sub.holes.end(), sub_holes.begin(), sub_holes.end());
            
            sub_outer = new_sub.outer;
            sub_holes = new_sub.holes;
            
            remains.push_back(*i);
            
            //I wish I could avoid this pattern.
            //Anyway, this removes the current hole. As the increment is sometimes performed here, it can't be in the for-loop.
            Paths::iterator temp = i;
            i++;
            a.holes.erase(temp);
            continue;
        }
        i++;
    }

    //Next, intersect the subtractor holes with a's outer path, and subtract a's holes from the result
    //This yields the 'islands'
    for(Paths::iterator hole = sub_holes.begin(); hole != sub_holes.end(); hole++) {
        Shapes temp = path_boolean(INTERSECT, a.outer, hole->reverse());
        std::list<Shape> new_islands = std::list<Shape>(temp.begin(), temp.end());
        bool on_remains = false;
        for(Paths::iterator i = a.holes.begin(); ; i++) {  //iterate a's holes / remains
            if(i == a.holes.end()) { i = remains.begin(); on_remains = true; }
            if(i == remains.end()) break;
            
            //We've already culled out the intersectors in the above loop
            if(on_remains || contains(sub_outer, i->initialPoint())) {
                for(std::list<Shape>::iterator j = new_islands.begin(); j != new_islands.end(); ) { // iterate the islands
                    //since the holes are disjoint, we don't need to do a recursive shape_subtract
                    Crossings hcr = crossings(j->outer, *i);
                    //TODO: use crosses predicate
                    if(!hcr.empty()) {
                        Shapes split = path_subtract_reverse(j->outer, *i);
                        add_holes(split, j->holes);
                        new_islands.insert(j, split.begin(), split.end());
                        
                        std::list<Shape>::iterator temp = j;
                        j++;
                        new_islands.erase(temp);
                        continue;
                    } else if(contains(j->outer, i->initialPoint())) {
                        j->holes.push_back(*i);
                    } else if(contains(*i, j->outer.initialPoint())) {
                        std::list<Shape>::iterator temp = j;
                        j++;
                        new_islands.erase(temp);
                        continue;
                    }
                    j++;
                }
            }
        }
        ret.insert(ret.end(), new_islands.begin(), new_islands.end());
    }
    
    //remove a's holes which are within the subtractor
    for(Paths::iterator i = a.holes.begin(); i != a.holes.end(); ) {
        if(contains(sub_outer, i->initialPoint())) {
            Paths::iterator temp = i;
            i++;
            a.holes.erase(temp);
            continue;
        }
        i++;
    }
    
    std::cout << "vier\n";
    if(flag_inside) {
        a.holes.push_back(sub_outer);
        ret.push_back(a);
    } else {
        Shapes outers = path_subtract_reverse(a.outer, sub_outer);
        add_holes(outers, a.holes);
        ret.insert(ret.end(), outers.begin(), outers.end());
    }
    
    return ret;
}

/*
Shapes shape_subtract(Shape const & a, Shape const & b) {
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
    std::cout << "ello\n";
    Paths inters = path_intersect(a.outer, b.outer, cr_a, cr_b);
    Shapes returns;
    for(Paths::iterator inter = inters.begin(); inter != inters.end(); inter++) {
        //Appends to the result the holes in the subtractor that are within the intersection
        for ( Paths::iterator holei = holes[1].begin();
                holei != holes[1].end(); holei++ ) {
            Crossings hcr = crossings(*inter, *holei);
            //TODO: use crosses predicate
            if(hcr.empty()) {
                if(contains(*inter, holei->initialPoint())) {
                    Shape s;
                    s.outer = *holei;
                    ret.push_back(s);
                }
            } else {
                hcr = reverse_crossings_b(hcr, holei->size());
                CrossingsA hcr_a(hcr.begin(), hcr.end());
                CrossingsB hcr_b(hcr_a.begin(), hcr_a.end());
                Shapes ps = path_boolean(INTERSECT, *inter, (*holei).reverse(), hcr_a, hcr_b);
                ret.insert(ret.end(), ps.begin(), ps.end());
            }
        }
        //Subtract A's holes from the result shapes
        for ( Paths::iterator j = holes[0].begin();
              j != holes[0].end(); j++ ) {
            for ( unsigned i = 0; i < ret.size(); i++) {

                Shape s; 
                s.outer = *j;
                Shapes ps = shape_subtract(ret[i], s);
                returns.insert(returns.end(), ps.begin(), ps.end());
            }
        }
    }
    return returns;
}
*/
Shapes shape_intersect(Shape const & a, Shape const & b) {
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
                s.outer = i->reverse();
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

Shapes path_boolean(BoolOp btype, Path const & a, Path const & b, Crossings const & cr) {
    CrossingsA cr_a(cr.begin(), cr.end());
    CrossingsB cr_b(cr_a.begin(), cr_a.end());
    return path_boolean(btype, a, b, cr_a, cr_b);
}

Shapes path_boolean_reverse(BoolOp btype, Path const & a, Path const & b, Crossings const &cr) {
    Crossings new_cr;
    double max = b.size();
    for(Crossings::const_iterator i = cr.begin(); i != cr.end(); i++) {
        Crossing x = *i;
        if(x.tb > max) x.tb = 1 - (x.tb - max) + max; // on the last seg - flip it about
        else x.tb = max - x.tb;
        x.dir = !x.dir;
        new_cr.push_back(x);
    }
    return path_boolean(btype, a, b.reverse(), new_cr);
}

unsigned outer_index(std::vector<Path> const &ps) {
    unsigned ix = 0;
    if(ps.size() == 1 || contains(ps[0], ps[1].initialPoint())) {
        return ix;
    } else {
        /* Since we've already shown that chunks[0] is not the outer_path,
           it can be used as an exemplar inner. */
        Point exemplar = ps[0].initialPoint();
        for(unsigned i = 1; i < ps.size(); i++) {
            if(contains(ps[i], exemplar)) {
                ix = i;
                break;
            }
        }
    }
    return ix;
}

bool logical_xor (bool a, bool b) { return (a || b) && !(a && b); }

/* This handles all the boolean ops in one function.  The middle function
 * is the main area of similarity between each.  The initial and ending
 * code is custom for each. */
Shapes path_boolean(BoolOp btype,
                    Path const & a, Path const & b,
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
                s.holes.push_back(b.reverse());
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
            if(logical_xor(it -> dir, btype != UNION)) {
                next = circ<CrossIterator>(cr_a.begin(), cr_a.end(), cr_a.find(*it));
                if(next == cr_a.end()) goto aus; //break;  //really these should never happen...
                next++;
                a.appendPortionTo(res, it->ta, (*next).ta);
            } else {
                next = circ<CrossIterator>(cr_b.begin(), cr_b.end(), cr_b.find(*it));
                if(next == cr_b.end()) goto aus; //break;
                next++;
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
        CrossIterator temp = it;
        it++;
        cr_a.erase(*temp);
        cr_b.erase(*temp);
    }
    aus:

    //Process the chunks into shapes output
    
    if(chunks.empty()) { return ret; }
    
    //If we are doing a union, the result may have multiple holes
    if(btype == UNION) {
        unsigned ix = outer_index(chunks);

        Shape s;
        s.outer = chunks[ix];
        for(unsigned i = 0; i < chunks.size(); i++) {        std::cout << "allo\n";
            /* If everything is functioning properly, these should all
             * have clockwise winding. TODO: stick some assertions in here*/
            if(i != ix && contains(s.outer, chunks[i].initialPoint()))
                s.holes.push_back(chunks[i]);
        }
        ret.push_back(s);
    } else {
        // Intersection and non-inclusion subtraction will result in disjoint paths
        return paths_to_shapes(chunks);
    }
    return ret;
}

}
