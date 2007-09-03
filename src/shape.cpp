#include "shape.h"
#include "utils.h"
#include "sweep.h"
#include "ord.h"

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

void add_to_shape(Shape &s, Path const &p, bool fill) {
    if(fill)
        s.content.push_back(Region(p).asFill());
    else
        s.content.push_back(Region(p).asHole());
}

/*
Shape sanitize(std::vector<Path> const & ps) {
    CrossingSet crs(crossings_among(ps));
    Shape ret;
    StatusSet stat;
    for(unsigned i = 0; i < crs.size(); i++)
        stat.push_back(std::vector<CrStatus>(crs[i].size(), CrStatus(0)));
    inner_sanitize(ret, ps, crs, stat);
    for(unsigned i = 0; i < crs.size(); i++) {
        if(crs[i].empty()) add_to_shape(ret, ps[i], ret.contains(Path(ps[i]).initialPoint()));
    }
    return ret;
} ?*/

unsigned pick_coincident(unsigned ix, unsigned jx, bool &rev, std::vector<Path> const &ps, CrossingSet const &crs) {
    unsigned ex_jx = jx;
    unsigned oix = crs[ix][jx].getOther(ix);
    double otime = crs[ix][jx].getTime(oix);
    Point cross_point = ps[oix].pointAt(otime),
          along = ps[oix].pointAt(otime + (rev ? -0.01 : 0.01)) - cross_point,
          prev = -along;
    bool ex_dir = rev;
    for(unsigned k = jx; k < crs[ix].size(); k++) {
        unsigned koix = crs[ix][k].getOther(ix);
        if(koix == oix) {
            if(!near(otime, crs[ix][k].getTime(oix))) break;
            for(unsigned dir = 0; dir < 2; dir++) {
                Point val = ps[ix].pointAt(crs[ix][k].getTime(ix) + (dir ? -0.01 : 0.01)) - cross_point;
                Cmp to_prev = cmp(cross(val, prev), 0);
                Cmp from_along = cmp(cross(along, val), 0);
                Cmp c = cmp(from_along, to_prev);
                if(c == EQUAL_TO && from_along == LESS_THAN) {
                    ex_jx = k;
                    prev = val;
                    ex_dir = dir;
                }
            }
        }
    }
    rev = ex_dir;
    return ex_jx;
}

unsigned crossing_along(double t, unsigned ix, unsigned jx, bool dir, Crossings const & crs) {
    Crossing cur = Crossing(t, t, ix, ix, false);
    if(jx < crs.size()) {
        double ct = crs[jx].getTime(ix);
        if( t == ct) cur = crs[jx];
    }
    if(!dir) {
        jx = std::upper_bound(crs.begin(), crs.end(), cur, CrossingOrder(ix)) - crs.begin();

    } else {
        jx = std::lower_bound(crs.begin(), crs.end(), cur, CrossingOrder(ix)) - crs.begin();
        if(jx == 0) jx = crs.size() - 1; else jx--;
        jx = std::lower_bound(crs.begin(), crs.end(), crs[jx], CrossingOrder(ix)) - crs.begin();
    }
    if(jx >= crs.size()) jx = 0;
    return jx;
}

void crossing_dual(unsigned &i, unsigned &j, CrossingSet const & crs) {
    Crossing cur = crs[i][j];
    i = cur.getOther(i);
    if(crs[i].empty())
        j = 0;
    else
        j = std::lower_bound(crs[i].begin(), crs[i].end(), cur, CrossingOrder(i)) - crs[i].begin();
}

//locate a crossing on the outside, by casting a ray through the middle of the bbox
void outer_crossing(unsigned &ix, unsigned &jx, bool & dir, std::vector<Path> const & ps, CrossingSet const & crs) {
    double ry = ps[ix].boundsFast()[Y].middle();
    double max_val = ps[ix].initialPoint()[X], max_t = 0;
    ix = ps.size();
    for(unsigned i = 0; i < ps.size(); i++) {
        if(!crs[i].empty()) {
            std::vector<double> rts = ps[i].roots(ry, Y);
            for(unsigned j = 0; j < rts.size(); j++) {
                double val = ps[i].valueAt(rts[j], X);
                if(val > max_val) {
                    ix = i;
                    max_val = val;
                    max_t = rts[j];
                }
            }
        }
    }
    if(ix != ps.size()) {
        dir = ps[ix].valueAt(max_t + 0.01, Y) >
              ps[ix].valueAt(max_t - 0.01, Y);
        jx = crossing_along(max_t, ix, jx, dir, crs[ix]);
    }
}

void inner_sanitize(Shape &ret, std::vector<Path> const & ps, unsigned depth = 0) {
    std::cout << depth << "\n";
    CrossingSet crs(crossings_among(ps));
    
    Regions chunks;
    
    std::vector<bool> used_path(ps.size(), false);
    std::vector<std::vector<bool> > visited, inner_dir;
    for(unsigned i = 0; i < crs.size(); i++) {
        visited.push_back(std::vector<bool>(crs[i].size(), false));
        inner_dir.push_back(std::vector<bool>(crs[i].size(), 2));
    }
    
    while(true) {
        unsigned ix = 0, jx = 0;
        bool dir = false;
        //find an outer crossing by trying various paths and checking if the crossings are used
        for(; ix < crs.size(); ix++) {
            //TODO: optimize so it doesn't unecessarily check stuff
            bool cont = true;
            for(unsigned j = 0; j < crs[ix].size(); j++) {
                if(!visited[ix][j]) { cont = false; break; }
            }
            if(cont) continue;
            unsigned rix = ix, rjx = jx;
            outer_crossing(rix, rjx, dir, ps, crs);
            if(rix == crs.size() || visited[rix][rjx]) continue;
            ix = rix; jx = rjx;
            break;
        }
        if(ix == crs.size()) break;
        
        crossing_dual(ix, jx, crs);

        dir = !dir;
        inner_dir[ix][jx] = dir;

        Path res;
        do {
            visited[ix][jx] = true;
            unsigned fix = ix, fjx = jx;
            
            bool new_dir = dir;
            
            jx = crossing_along(crs[ix][jx].getTime(ix), ix, jx, dir, crs[ix]);
            crossing_dual(ix, jx, crs);
            jx = pick_coincident(ix, jx, new_dir, ps, crs);
            
            unsigned nix = ix, njx = jx;
            crossing_dual(nix, njx, crs);
            
            inner_dir[fix][fjx] = inner_dir[nix][njx] = dir;
            
            Crossing from = crs[fix][fjx],
                     to = crs[ix][jx];
            if(dir) {
                // backwards
                std::cout << "r" << ix << "[" << to.getTime(ix)  << ", " << from.getTime(ix) << "]\n";
                Path p = ps[ix].portion(to.getTime(ix), from.getTime(ix)).reverse();
                for(unsigned i = 0; i < p.size(); i++)
                    res.append(p[i]);
            } else {
                // forwards
                std::cout << "f" << ix << "[" << from.getTime(ix) << ", " << to.getTime(ix) << "]\n";
                ps[ix].appendPortionTo(res, from.getTime(ix), to.getTime(ix));
            }
            dir = new_dir;
        } while(!visited[ix][jx]);
        std::cout << "added " << res.size() << "\n";
        add_to_shape(ret, res, depth%2==0);
        //break;
        std::vector<Path> nxt;
        std::vector<std::vector<bool> > visited2;
        for(unsigned i = 0; i < crs.size(); i++)
            visited2.push_back(std::vector<bool>(crs[i].size(), false));
        while(true) {
            ix = 0; jx = 0;
            for(; ix < crs.size(); ix++)
                for(jx = 0; jx < crs[ix].size(); jx++)
                    if(visited[ix][jx] && !visited2[ix][jx]) goto found;
            found:
            if(ix == crs.size()) break;
            
            dir = inner_dir[ix][jx];
            
            Path res2;
            do {
                visited[ix][jx] = visited2[ix][jx] = true;
                unsigned fix = ix, fjx = jx;
                if(visited[ix][jx]) {
                    crossing_dual(ix, jx, crs);
                    jx = crossing_along(crs[ix][jx].getTime(ix), ix, jx, dir, crs[ix]);
                    
                    dir = inner_dir[ix][jx];
                    if(dir == 2) std::cout << "unset\n";
                } else {
                    jx = crossing_along(crs[ix][jx].getTime(ix), ix, jx, dir, crs[ix]);
                }
                
                Crossing from = crs[fix][fjx],
                         to = crs[ix][jx];
                if(dir) {
                    // backwards
                    std::cout << "r" << ix << "[" << to.getTime(ix)  << ", " << from.getTime(ix) << "]\n";
                    Path p = ps[ix].portion(to.getTime(ix), from.getTime(ix)).reverse();
                    for(unsigned i = 0; i < p.size(); i++)
                        res2.append(p[i]);
                } else {
                    // forwards
                    std::cout << "f" << ix << "[" << from.getTime(ix) << ", " << to.getTime(ix) << "]\n";
                    ps[ix].appendPortionTo(res2, from.getTime(ix), to.getTime(ix));
                }
            } while(!visited2[ix][jx]);
            add_to_shape(ret, res2, depth%2==1);
            //nxt.push_back(res2);
            break;
        }
        std::cout << "\n";
        //inner_sanitize(ret, nxt, depth+1);
        
        break;
    }
    for(unsigned i = 0; i < crs.size(); i++) {
        if(crs[i].empty() && !used_path[i])
            add_to_shape(ret, ps[i], depth%2==0);
    }
}

Shape sanitize(std::vector<Path> const & ps) {
    Shape ret;
    inner_sanitize(ret, ps);
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
