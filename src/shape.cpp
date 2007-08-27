#include "shape.h"
#include "utils.h"
#include "sweep.h"

#include <iostream>
#include <algorithm>

namespace Geom {

//Utility funcs
//yes, xor is !=, but I'm pretty sure this is safer in the event of strange bools
bool logical_xor (bool a, bool b) { return (a || b) && !(a && b); }

bool disjoint(Path const & a, Path const & b) {
    return !contains(a, b.initialPoint()) && !contains(b, a.initialPoint());
}

template<typename T>
void append(T &a, T const &b) {
    a.insert(a.end(), b.begin(), b.end());
}

void first_false(std::vector<std::vector<bool> > visited, unsigned &i, unsigned &j) {
    for(i = 0, j = 0; i < visited.size(); i++) {
        std::vector<bool>::iterator unvisited = std::find(visited[i].begin(), visited[i].end(), false);
        if(unvisited != visited[i].end()) {
            j = unvisited - visited[i].begin();
            break;
        }
    }
}

unsigned find_crossing(Crossings const &cr, Crossing x, unsigned i) {
    return std::lower_bound(cr.begin(), cr.end(), x, CrossingOrder(i)) - cr.begin();
}

/* This function handles boolean ops on shapes.  The first parameter is a bool
 * which determines its behavior in each combination of these cases.  For proper
 * fill information and noncrossing behavior, the fill data of the regions must
 * be correct.  The boolean parameter determines whether the operation is a
 * union or a subtraction.  Reversed paths represent inverse regions, where
 * everything is included in the fill except for the insides.
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
 * The operation of this function isn't very complicated.  It just traverses the crossings, and uses the crossing direction to decide whether the next segment should be from A or from B.
 */
Shape shape_boolean(bool rev, Shape const & a, Shape const & b, CrossingSet const & crs) {
    const Regions ac = a.content, bc = b.content;

    //Keep track of which crossings we've hit.
    std::vector<std::vector<bool> > visited;
    for(unsigned i = 0; i < crs.size(); i++)
        visited.push_back(std::vector<bool>(crs[i].size(), false));

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
    
    //If true, then the hole must be inside the other to be included
    bool const on_sub = logical_xor(a.fill, b.fill);
    bool const a_mode = logical_xor(logical_xor(!rev, a.fill), on_sub),
               b_mode = logical_xor(logical_xor(!rev, b.fill), on_sub);
    //Handle unintersecting portions
    for(unsigned i = 0; i < crs.size(); i++) {
        if(crs[i].size() == 0) {
            Region    r(i < ac.size() ? ac[i] : bc[i - ac.size()]);
            bool mode = i < ac.size() ? a_mode : b_mode;
            
            if(logical_xor(r.fill, i < ac.size() ? a.fill : b.fill)) {
                //is an inner
                Point exemplar = r.boundary.initialPoint();
                Regions const & others = i < ac.size() ? bc : ac;
                for(unsigned j = 0; j < others.size(); j++) {
                    if(others[j].contains(exemplar)) {
                        //contained in other
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

std::vector<Rect> Shape::bounds() const {
    std::vector<Rect> rs;
    for(unsigned i = 0; i < size(); i++) rs.push_back(content[i].boundsFast());
    return rs; 
}

//Returns a vector of crossings, such that those associated with B are in the range [a.size(), a.size() + b.size())
CrossingSet crossings_between(Shape const &a, Shape const &b) { 
    CrossingSet results(a.size() + b.size(), Crossings());
    
    std::vector<std::vector<unsigned> > cull = sweep_bounds(a.bounds(), b.bounds());
    for(unsigned i = 0; i < cull.size(); i++) {
        for(unsigned jx = 0; jx < cull[i].size(); jx++) {
            std::cout << i << " " << jx << "\n";
            unsigned j = cull[i][jx];
            unsigned jc = j + a.size();
            Crossings cr = crossings(a.content[i].getBoundary(), b.content[j].getBoundary());
            for(unsigned k = 0; k < cr.size(); k++) { cr[k].a = i; cr[k].b = jc; }
            
            //Sort & add A-sorted crossings
            sort_crossings(cr, i);
            Crossings n(results[i].size() + cr.size());
            std::merge(results[i].begin(), results[i].end(), cr.begin(), cr.end(), n.begin(), CrossingOrder(i));
            results[i] = n;
            
            //Sort & add B-sorted crossings
            sort_crossings(cr, jc);
            n.resize(results[jc].size() + cr.size());
            std::merge(results[jc].begin(), results[jc].end(), cr.begin(), cr.end(), n.begin(), CrossingOrder(jc));
            results[jc] = n;
        }
    }
    return results;
}

Shape shape_boolean(bool rev, Shape const & a, Shape const & b) {
    CrossingSet crs = crossings_between(a, b);
    
    return shape_boolean(rev, a, b, crs);
}

Shape shape_boolean(Shape const &a, Shape const &b, unsigned flags) {
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
            }
            case BOOLOP_UNION:        return shape_boolean(false, a, b);
        }
    } else {
        switch(flags - BOOLOP_NEITHER) {
            case BOOLOP_SUBTRACT_A_B: return shape_boolean(false, b, a.inverse());
            case BOOLOP_SUBTRACT_B_A: return shape_boolean(false, a, b.inverse());
            case BOOLOP_EXCLUSION: {
                Shape res = shape_boolean(false, b, a.inverse());
                append(res.content, shape_boolean(false, a, b.inverse()).content);
                return res;
            }
        }
        return shape_boolean(a, b, ~flags).inverse();
    }
    return Shape();
}

std::vector<double> region_sizes(Shape const &a) {
    std::vector<double> ret;
    for(unsigned i = 0; i < a.size(); i++)
        ret.push_back(a[i].size());
    return ret;
}

Shape shape_boolean_ra(bool rev, Shape const &a, Shape const &b, CrossingSet const &crs) {
    return shape_boolean(rev, a.inverse(), b, reverse_ta(crs, a.size(), region_sizes(a)));
}

Shape shape_boolean_rb(bool rev, Shape const &a, Shape const &b, CrossingSet const &crs) {
    return shape_boolean(rev, a, b.inverse(), reverse_ta(crs, a.size(), region_sizes(b)));
}

Shape shape_boolean(Shape const &a, Shape const &b, unsigned flags, CrossingSet const &crs) {
    flags &= 15;
    if(flags <= BOOLOP_UNION) {
        switch(flags) {
            case BOOLOP_INTERSECT:    return shape_boolean(true, a, b, crs);
            case BOOLOP_SUBTRACT_A_B: return shape_boolean_rb(true, a, b, crs);
            case BOOLOP_IDENTITY_A:   return a;
            case BOOLOP_SUBTRACT_B_A: return shape_boolean_ra(true, a, b, crs);
            case BOOLOP_IDENTITY_B:   return b;
            case BOOLOP_EXCLUSION: {
                Shape res = shape_boolean_rb(true, a, b, CrossingSet(crs));
                append(res.content, shape_boolean_ra(true, a, b, CrossingSet(crs)).content);
                return res;
            }
            case BOOLOP_UNION:        return shape_boolean(false, a, b);
        }
    } else {
        switch(flags - BOOLOP_NEITHER) {
            case BOOLOP_SUBTRACT_A_B: return shape_boolean_ra(false, a, b, CrossingSet(crs));
            case BOOLOP_SUBTRACT_B_A: return shape_boolean_rb(false, a, b, CrossingSet(crs));
            case BOOLOP_EXCLUSION: {
                Shape res = shape_boolean_ra(false, a, b, CrossingSet(crs));
                append(res.content, shape_boolean_rb(false, a, b, CrossingSet(crs)).content);
                return res;
            }
        }
        return shape_boolean(a, b, ~flags).inverse();
    }
    return Shape();
}

int paths_winding(std::vector<Path> const &ps, Point p) {
    int ret;
    for(unsigned i = 0; i < ps.size(); i++)
        ret += winding(ps[i], p);
    return ret;
}

//sanitize
//We have two phases, one for each winding direction.
Shape sanitize_paths(std::vector<Path> const &ps, bool evenodd) {
    CrossingSet crs;// = crossings_among(ps);
    
    //two-phase process - g
    Regions chunks;
    for(bool phase = 0; phase < 2; phase++) {
        
        //Keep track of which crossings we've hit.
        std::vector<std::vector<bool> > visited;
        for(unsigned i = 0; i < crs.size(); i++)
            visited.push_back(std::vector<bool>(crs[i].size(), false));

        while(true) {
            unsigned i, j;
            first_false(visited, i, j);
            if(i == visited.size()) break;
            
            bool use = paths_winding(ps, ps[i].initialPoint()) % 2 == 1;
            Path res;
            do {
                Crossing cur = crs[i][j];
                visited[i][j] = true;
                
                //get indices of the dual:
                i = cur.getOther(i), j = find_crossing(crs[i], cur, i);
                if(j < visited[i].size()) visited[i][j] = true;
                
                if(logical_xor(phase, cur.dir)) {
                    // forwards
                    j++;
                    if(j >= crs[i].size()) j = 0;
                } else {
                    // backwards
                    if(j == 0) j = crs[i].size() - 1; else j--;
                }
                if(use) {
                    Crossing next = crs[i][j];
                    ps[i].appendPortionTo(res, cur.ta, next.ta);
                }
            } while(!visited[i][j]);
            
            if(use) {
                chunks.push_back(Region(res, true));
            }
        }
    }
    return Shape(chunks);
}

Shape Shape::operator*(Matrix const &m) const {
    Shape ret;
    for(unsigned i = 0; i < size(); i++)
        ret.content.push_back(content[i] * m);
    ret.fill = fill;
    return ret;
}

// inverse is a boolean not
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
    std::vector<std::vector<unsigned> > cull = sweep_bounds(pnt, bounds());
    if(cull[0].size() == 0) return !fill;
    return content[*min_element(cull[0].begin(), cull[0].end(), ContainmentOrder(&content))].isFill();
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
