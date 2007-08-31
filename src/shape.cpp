#include "shape.h"
#include "utils.h"
#include "sweep.h"

#include <iostream>
#include <algorithm>

namespace Geom {

// Utility funcs

// Yes, xor is !=, but I'm pretty sure this is safer in the event of strange bools
bool logical_xor (bool a, bool b) { return (a || b) && !(a && b); }

// A little sugar for appending a list to another
template<typename T>
void append(T &a, T const &b) {
    a.insert(a.end(), b.begin(), b.end());
}

/* Used within shape_boolean and related functions, as the name describes, finds the
 * first false within the list of lists of booleans.
 */
void first_false(std::vector<std::vector<bool> > visited, unsigned &i, unsigned &j) {
    for(i = 0, j = 0; i < visited.size(); i++) {
        std::vector<bool>::iterator unvisited = std::find(visited[i].begin(), visited[i].end(), false);
        if(unvisited != visited[i].end()) {
            j = unvisited - visited[i].begin();
            break;
        }
    }
}

// Finds a crossing in a list of them, given the sorting index.
unsigned find_crossing(Crossings const &cr, Crossing x, unsigned i) {
    return std::lower_bound(cr.begin(), cr.end(), x, CrossingOrder(i)) - cr.begin();
}

/* This function handles boolean ops on shapes.  The first parameter is a bool
 * which determines its behavior in each combination of cases.  For proper
 * fill information and noncrossing behavior, the fill data of the regions
 * must be correct.  The boolean parameter determines whether the operation
 * is a union or a subtraction.  Reversed paths represent inverse regions,
 * where everything is included in the fill except for the insides.
 *
 * Here is a chart of the behavior under various circumstances:
 * 
 * rev = false (union)
 *            A
 *        F         H
 * F  A+B -> F  A-B -> H
 *B
 * H  B-A -> H  AxB -> H
 *
 * rev = true (intersect)
 *            A
 *        F         H
 * F  AxB -> F  B-A -> F
 *B
 * H  A-B -> F  A+B -> H
 *
 * F/H = Fill outer / Hole outer
 * A/B specify operands
 * + = union, - = subtraction, x = intersection
 * -> read as "produces"
 *
 * This is the main function of boolops, yet its operation isn't very complicated.
 * It traverses the crossings, and uses the crossing direction to decide whether
 * the next segment should be taken from A or from B.  The second half of the
 * function deals with figuring out what to do with bits that have no intersection.
 */
Shape shape_boolean(bool rev, Shape const & a, Shape const & b, CrossingSet const & crs) {
    const Regions ac = a.content, bc = b.content;

    //Keep track of which crossings we've hit.
    std::vector<std::vector<bool> > visited;
    for(unsigned i = 0; i < crs.size(); i++)
        visited.push_back(std::vector<bool>(crs[i].size(), false));
    
    //bool const exception = 
    
    //Traverse the crossings, creating chunks
    Regions chunks;
    while(true) {
        unsigned i, j;
        first_false(visited, i, j);
        if(i == visited.size()) break;
        
        Path res;
        do {
            Crossing cur = crs[i][j];
            visited[i][j] = true;
            
            //get indices of the dual:
            unsigned io = cur.getOther(i), jo = find_crossing(crs[io], cur, io);
            if(jo < visited[io].size()) visited[io][jo] = true;
            
            //main driving logic
            if(logical_xor(cur.dir, rev)) {
                if(i >= ac.size()) { i = io; j = jo; }
                j++;
                if(j >= crs[i].size()) j = 0;
                Crossing next = crs[i][j];
                ac[next.a].boundary.appendPortionTo(res, cur.ta, next.ta);
            } else {
                if(i < ac.size()) { i = io; j = jo; }
                j++;
                if(j >= crs[i].size()) j = 0;
                Crossing next = crs[i][j];
                bc[next.b - ac.size()].boundary.appendPortionTo(res, cur.tb, next.tb);
            }
        } while (!visited[i][j]);
        if(res.size() > 0) chunks.push_back(Region(res));
    }
    
    //If true, then we are on the 'subtraction diagonal'
    bool const on_sub = logical_xor(a.fill, b.fill);
    //If true, then the hole must be inside the other to be included
    bool const a_mode = logical_xor(logical_xor(!rev, a.fill), on_sub),
               b_mode = logical_xor(logical_xor(!rev, b.fill), on_sub);
    
    //Handle unintersecting portions
    for(unsigned i = 0; i < crs.size(); i++) {
        if(crs[i].size() == 0) {
            Region    r(i < ac.size() ? ac[i] : bc[i - ac.size()]);
            bool   mode(i < ac.size() ? a_mode : b_mode);
            
            if(logical_xor(r.fill, i < ac.size() ? a.fill : b.fill)) {
                //is an inner (fill is opposite the outside fill)
                Point exemplar = r.boundary.initialPoint();
                Regions const & others = i < ac.size() ? bc : ac;
                for(unsigned j = 0; j < others.size(); j++) {
                    if(others[j].contains(exemplar)) {
                        //contained in another
                        if(mode) chunks.push_back(r);
                        goto skip;
                    }
                }
            }
            //disjoint
            if(!mode) chunks.push_back(r);
            skip: (void)0;
        }
    }
    
    return Shape(chunks);
}

// Just a convenience wrapper for shape_boolean, which handles the crossings
Shape shape_boolean(bool rev, Shape const & a, Shape const & b) {
    CrossingSet crs = crossings_between(a, b);
    
    return shape_boolean(rev, a, b, crs);
}


// Some utility functions for boolop:

std::vector<double> region_sizes(Shape const &a) {
    std::vector<double> ret;
    for(unsigned i = 0; i < a.size(); i++) {
        ret.push_back(double(a[i].size()));
    }
    return ret;
}

Shape shape_boolean_ra(bool rev, Shape const &a, Shape const &b, CrossingSet const &crs) {
    return shape_boolean(rev, a.inverse(), b, reverse_ta(crs, a.size(), region_sizes(a)));
}

Shape shape_boolean_rb(bool rev, Shape const &a, Shape const &b, CrossingSet const &crs) {
    return shape_boolean(rev, a, b.inverse(), reverse_tb(crs, a.size(), region_sizes(b)));
}

/* This is a function based on shape_boolean which allows boolean operations
 * to be specified as a logic table.  This logic table is 4 bit-flags, which
 * correspond to the elements of the 'truth table' for a particular operation.
 * These flags are defined with the enums starting with BOOLOP_ .
 */
Shape boolop(Shape const &a, Shape const &b, unsigned flags, CrossingSet const &crs) {
    flags &= 15;
    if(flags <= BOOLOP_UNION) {
        switch(flags) {
            case BOOLOP_INTERSECT:    return shape_boolean(true, a, b, crs);
            case BOOLOP_SUBTRACT_A_B: return shape_boolean_rb(true, a, b, crs);
            case BOOLOP_IDENTITY_A:   return a;
            case BOOLOP_SUBTRACT_B_A: return shape_boolean_ra(true, a, b, crs);
            case BOOLOP_IDENTITY_B:   return b;
            case BOOLOP_EXCLUSION: {
                Shape res = shape_boolean_rb(true, a, b, crs);
                append(res.content, shape_boolean_ra(true, a, b, crs).content);
                return res;
            }
            case BOOLOP_UNION:        return shape_boolean(false, a, b);
        }
    } else {
        switch(flags - BOOLOP_NEITHER) {
            case BOOLOP_SUBTRACT_A_B: return shape_boolean_ra(false, a, b, crs);
            case BOOLOP_SUBTRACT_B_A: return shape_boolean_rb(false, a, b, crs);
            case BOOLOP_EXCLUSION: {
                Shape res = shape_boolean_ra(false, a, b, CrossingSet(crs));
                append(res.content, shape_boolean_rb(false, a, b, CrossingSet(crs)).content);
                return res;
            }
        }
        return boolop(a, b, ~flags, crs).inverse();
    }
    return Shape();
}

/* This version of the boolop function doesn't require a set of crossings, as
 * it computes them for you.  This is more efficient in some cases, as the
 * shape can be inverted before finding crossings.  In the special case of
 * exclusion it uses the other version of boolop.
 */
Shape boolop(Shape const &a, Shape const &b, unsigned flags) {
    flags &= 15;
    if(flags <= BOOLOP_UNION) {
        switch(flags) {
            case BOOLOP_INTERSECT:    return shape_boolean(true, a, b);
            case BOOLOP_SUBTRACT_A_B: return shape_boolean(true, a, b.inverse());
            case BOOLOP_IDENTITY_A:   return a;
            case BOOLOP_SUBTRACT_B_A: return shape_boolean(true, b, a.inverse());
            case BOOLOP_IDENTITY_B:   return b;
            case BOOLOP_EXCLUSION: {
                Shape res = shape_boolean(true, a, b.inverse());
                append(res.content, shape_boolean(true, b, a.inverse()).content);
                return res;
            } //return boolop(a, b, flags,  crossings_between(a, b));
            case BOOLOP_UNION:        return shape_boolean(false, a, b);
        }
    } else {
        switch(flags - BOOLOP_NEITHER) {
            case BOOLOP_SUBTRACT_A_B: return shape_boolean(false, b, a.inverse());
            case BOOLOP_SUBTRACT_B_A: return shape_boolean(false, a, b.inverse());
            case BOOLOP_EXCLUSION: {
                Shape res = shape_boolean(false, a, b.inverse());
                append(res.content, shape_boolean(false, b, a.inverse()).content);
                return res;
            } //return boolop(a, b, flags, crossings_between(a, b));
        }
        return boolop(a, b, ~flags).inverse();
    }
    return Shape();
}

int paths_winding(std::vector<Path> const &ps, Point p) {
    int ret = 0;
    for(unsigned i = 0; i < ps.size(); i++)
        ret += winding(ps[i], p);
    return ret;
}

void add_to_shape(Shape &s, Path const &p) {
    s.content.push_back(Region(p));
    /* if(s.contains(p.initialPoint()))
        s.content.push_back(Region(p).asHole());
    else
        s.content.push_back(Region(p).asFill());
    */
}

double point_sine(Point a, Point b, Point c) {
    Point db = b - a, dc = c - a;
    return cross(db, dc) / (db.length() * dc.length());
}

//Non-public, recursive function to turn paths into a shape
//Handles coincidence, yet not coincidence of derivative & crossing
void inner_sanitize(Shape &ret, std::vector<Path> const & ps, CrossingSet const & cr_in, unsigned depth = 0) {
    CrossingSet crs = cr_in;
    while(true) {
        //Find a path with crossings
        unsigned has_cross = 0;
        for(; has_cross < crs.size(); has_cross++) {
            if(!crs[has_cross].empty()) break;
        }
        if(has_cross == crs.size()) return;
        
        //locate a crossing on the outside, by casting a ray through the middle
        double ry = ps[has_cross].boundsFast()[Y].middle();
        unsigned max_ix = has_cross;
        double max_val = ps[has_cross].initialPoint()[X], max_t = 0;
        for(unsigned i = 0; i < ps.size(); i++) {
            if(!crs[i].empty()) {
                std::vector<double> rts = ps[i].roots(ry, Y);
                for(unsigned j = 0; j < rts.size(); j++) {
                    double val = ps[i].valueAt(rts[j], X);
                    if(val > max_val) {
                        max_ix = i;
                        max_val = val;
                        max_t = rts[j];
                    }
                }
            }
        }
        std::vector<Crossing>::iterator lb = std::lower_bound(crs[max_ix].begin(), crs[max_ix].end(),
                                                              Crossing(max_t, max_t, max_ix, max_ix, false), CrossingOrder(max_ix));
        unsigned i = max_ix, j = (lb == crs[max_ix].end()) ? 0 : lb - crs[max_ix].begin();
        if(crs[i][j].getTime(i) != max_t) j++;
        if(j >= crs[max_ix].size()) j = 0;
        Crossing cur = crs[i][j];
        
        //Keep track of which crossings we've hit.
        std::vector<std::vector<bool> > visited;
        for(unsigned i = 0; i < crs.size(); i++)
            visited.push_back(std::vector<bool>(crs[i].size(), false));
        
        //starting from this crossing, traverse the outer path
        Path res;
        bool rev = true;
        do {
            visited[i][j] = true;
            //std::cout << i << ", " << j << " -> ";
            //get indices of the next crossing:
            double otime = crs[i][j].getTime(i);
            Point pnt = ps[i].pointAt(otime);
            Point along = ps[i].pointAt(otime + (rev ? -0.01 : 0.01));
            
            i = cur.getOther(i);
            
            unsigned first = std::lower_bound(crs[i].begin(), crs[i].end(), cur, CrossingOrder(i)) - crs[i].begin();
            double first_time = crs[i][first].getTime(i);
            unsigned ex_ix = first;
            double ex_val = 0;
            bool ex_dir = false;
            for(unsigned k = first; k < crs[i].size() && near(first_time, crs[i][k].getTime(i)); k++) {
                if(!visited[i][k]) {
                    for(unsigned dir = 0; dir < 2; dir++) {
                        double val = point_sine(pnt, along, ps[i].pointAt(crs[i][k].getTime(i) + (dir ? -0.01 : 0.01)));
                        if(val > ex_val) {
                            ex_val = val; ex_ix = k; ex_dir = dir;
                        }
                    }
                }
            }

            j = ex_ix;
            rev = ex_dir;
            //std::cout << i << ", " << j << ": " << rev << "\n";
            double curt = cur.getTime(i);

            if(rev) {
                // backwards
                if(j == 0) j = crs[i].size() - 1; else j--;
                cur = crs[i][j];
                std::cout << "r" << i << "[" << cur.getTime(i) << ", " << curt << "]\n";
                Path p = ps[i].portion(cur.getTime(i), curt).reverse();
                for(unsigned k = 0; k < p.size(); k++)
                    res.append(p[k]);
            } else {
                // forwards
                j++;
                if(j >= crs[i].size()) j = 0;
                cur = crs[i][j];
                std::cout << "f" << i << "[" << curt << ", " << cur.getTime(i) << "]\n";
                ps[i].appendPortionTo(res, curt, cur.getTime(i));
            }
        
        } while(!visited[i][j]);
        
        add_to_shape(ret, res);
       
        CrossingSet new_crs(crs.size(), Crossings());
        CrossingSet inner_crs(crs.size(), Crossings());
        for(unsigned k = 0; k < crs.size(); k++) {
            for(unsigned l = 0; l < crs[k].size(); l++) {
                Crossing c = crs[k][l];
                if(!visited[k][l]) {
                    if(contains(res, ps[c.a].pointAt(c.ta))) inner_crs[k].push_back(c); else new_crs[k].push_back(c);
                }
            }
        }
        inner_sanitize(ret, ps, inner_crs, depth+1);
        crs = new_crs;
    }
}

Shape sanitize(std::vector<Path> const & ps) {
    CrossingSet crs(crossings_among(ps));
    Shape ret;
    inner_sanitize(ret, ps, crs);
    for(unsigned i = 0; i < crs.size(); i++) {
        if(crs[i].empty()) add_to_shape(ret, ps[i]);
    }
    return ret;
}

/* This transforms a shape by a matrix.  In the case that the matrix flips
 * the shape, it reverses the paths in order to preserve the fill.
 */
Shape Shape::operator*(Matrix const &m) const {
    Shape ret;
    for(unsigned i = 0; i < size(); i++)
        ret.content.push_back(content[i] * m);
    ret.fill = fill;
    return ret;
}

// Inverse is a boolean not, and simply reverses all the paths & fill flags
Shape Shape::inverse() const {
    Shape ret;
    for(unsigned i = 0; i < size(); i++)
        ret.content.push_back(content[i].inverse());
    ret.fill = !fill;
    return ret;
}

struct ContainmentOrder {
    std::vector<Region> const *rs;
    explicit ContainmentOrder(std::vector<Region> const *r) : rs(r) {}
    bool operator()(unsigned a, unsigned b) const { return (*rs)[b].contains((*rs)[a]); }
};

bool Shape::contains(Point const &p) const {
    std::vector<Rect> pnt;
    pnt.push_back(Rect(p, p));
    std::vector<std::vector<unsigned> > cull = sweep_bounds(pnt, bounds(*this));
    if(cull[0].size() == 0) return !fill;
    std::vector<unsigned> containers;
    for(unsigned i = 0; i < cull[0].size(); i++)
        if(content[cull[0][i]].contains(p)) containers.push_back(cull[0][i]);
    return content[*min_element(containers.begin(), containers.end(), ContainmentOrder(&content))].isFill();
}

bool Shape::inside_invariants() const {  //semi-slow & easy to violate
    for(unsigned i = 0; i < size(); i++)
        if( logical_xor(content[i].isFill(), contains(content[i].boundary.initialPoint())) ) return false;
    return true;
}
bool Shape::region_invariants() const { //semi-slow
    for(unsigned i = 0; i < size(); i++)
        if(!content[i].invariants()) return false;
    return true;
}
bool Shape::cross_invariants() const { //slow
    CrossingSet crs; // = crossings_among(paths_from_regions(content));
    for(unsigned i = 0; i < crs.size(); i++)
        if(!crs[i].empty()) return false;
    return true;
}

bool Shape::invariants() const {
    return inside_invariants() && region_invariants() && cross_invariants();
}

}
