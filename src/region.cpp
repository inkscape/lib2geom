#include "region.h"
#include "utils.h"

namespace Geom {

Region Region::operator*(Matrix const &m) const {
    return Region(_boundary * m, logical_xor(m.flips(), _fill));
}

Regions path_union(Region const & a, Region const & b, bool typ) {
    return region_boolean(!typ,
        logical_xor(a.fill(), typ) ? a.inverse() : a,
        logical_xor(b.fill(), typ) ? b.inverse() : b);
}

Regions path_subtract(Region const & a, Region const & b, bool typ) {
    return region_boolean(typ,
        logical_xor(a.fill(), typ) ? a.inverse() : a,
        logical_xor(b.fill(), !typ) ? b.inverse() : b);
}

Regions path_intersect(Region const & a, Region const & b, bool typ) {
    return region_boolean(typ,
        logical_xor(!a.fill(), typ) ? a.inverse() : a,
        logical_xor(!b.fill(), typ) ? b.inverse() : b);
}

Regions path_exclude(Region const & a, Region const & b, bool typ) {
    Regions ret = path_subtract(a, b, typ);
    Regions add = path_subtract(b, a, typ);
    ret.insert(ret.end(), add.begin(), add.end());
    return ret;
}

Regions region_boolean(bool btype, Region const & a, Region const & b, Crossings const & cr) {
    Crossings cr_a = cr, cr_b = cr;
    sort_crossings(cr_a, 0); sort_crossings(cr_b, 1);
    return region_boolean(btype, a, b, cr_a, cr_b);
}

unsigned outer_index(Regions const &ps) {
    if(ps.size() <= 1 || contains(ps[0].boundary(), ps[1].boundary().initialPoint())) {
        return 0;
    } else {
        /* Since we've already shown that chunks[0] is not outside
           it can be used as an exemplar inner. */
        Point exemplar = ps[0].boundary().initialPoint();
        for(unsigned i = 1; i < ps.size(); i++) {
            if(ps[i].contains(exemplar)) {
                std::cout << "oi: " << i << "\n";
                return i;
            }
        }
    }
    return ps.size();
}

//Returns a vector of crossings, such that those associated with B are in the range [a.size(), a.size() + b.size())
std::vector<Crossings> crossings_between(Regions const &a, Regions const &b) {
    std::vector<Crossings> results(a.size() + b.size(), Crossings());

    //TODO: sweep
    for(unsigned i = 0; i < a.size(); i++) {
        for(unsigned jx = 0; jx < b.size(); jx++) {
            unsigned j = jx + a.size();
            Crossings cr = crossings(a[i].boundary(), b[jx].boundary());
            for(unsigned k = 0; k < cr.size(); k++) { cr[k].a = i; cr[k].b = j; }
            //Sort & add I crossings
            sort_crossings(cr, i);
            Crossings n(results[i].size() + cr.size());
            std::merge(results[i].begin(), results[i].end(), cr.begin(), cr.end(), n.begin(), CrossingOrder(i));
            results[i] = n;
            //Sort & add J crossings
            sort_crossings(cr, j);
            n.resize(results[j].size() + cr.size());
            std::merge(results[j].begin(), results[j].end(), cr.begin(), cr.end(), n.begin(), CrossingOrder(j));
            results[j] = n;
        }
    }
    return results;
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

unsigned find_crossing(Crossings const &cr, Crossing x) {
    return std::find(cr.begin(), cr.end(), x) - cr.begin();
}

Regions regions_boolean(bool rev, Regions const & a, Regions const & b) {
    std::vector<Crossings> crs = crossings_between(a, b);
    
    /* for(unsigned i = 0; i < crs.size(); i++) {
        std::cout << i << "\n";
        for(unsigned j = 0; j < crs[i].size(); j++) {
            std::cout << " " << crs[i][j].a << " " << crs[i][j].b << " :" << crs[i][j].ta << " to " << crs[i][j].tb << "\n";
        }
    } */
    
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
        std::cout << "starting traverse on " << i << ", " << j << "\n";
        Path res;
        do {
            Crossing cur = crs[i][j];
            visited[i][j] = true;
            std::cout << i << ", " << j << ": " << (cur.dir ? "true" : "false") << "\n";
            if(logical_xor(cur.dir, rev)) {
                if(i >= a.size()) {
                    i = cur.a;
                    j = find_crossing(crs[i], cur);
                    visited[i][j] = true;
                }
                j++;
                if(j >= crs[i].size()) j = 0;
                Crossing next = crs[i][j];
                a[next.a]._boundary.appendPortionTo(res, cur.ta, next.ta);
            } else {
                if(i < a.size()) {
                    i = cur.b;
                    j = find_crossing(crs[i], cur);
                    visited[i][j] = true;
                }
                j++;
                if(j >= crs[i].size()) j = 0;
                Crossing next = crs[i][j];
                b[next.b - a.size()]._boundary.appendPortionTo(res, cur.tb, next.tb);

            }
            
        } while (!visited[i][j]);
        chunks.push_back(Region(res));
    }
    
    //Handle unintersecting portions
    for(unsigned i = 0; i < crs.size(); i++) {
        if(crs[i].size() == 0) {
            Region r = i < a.size() ? a[i] : b[i - a.size()];
            Point exemplar = r.boundary().initialPoint();
            for(unsigned j = 0; j < chunks.size(); j++) {
                if(chunks[j].contains(exemplar) && logical_xor(r.fill(), chunks[j].fill())) {
                    chunks.push_back(r);
                    goto skip;
                }
            }
            //disjoint
            if(r.fill() && !rev) {
                //and should be included
                chunks.push_back(r);
            }
            skip: (void)0;
        }
    }
    
    return chunks;
}

/* This function handles boolean ops on regions of fill or hole.  The first parameter is a bool
 * which determines its behavior in each combination of these cases.  For proper fill information
 * and noncrossing behavior, the fill data of the regions must be correct.  The boolean parameter
 * determines whether the operation is a union or a subtraction.  Reversed paths represent inverse
 * regions, where everything is included in the fill except for the insides.  Here is a chart of
 * the behavior under various circumstances:
 * 
 * rev = false
 *            A
 *       F       H
 * F  A+B->FH  A-B->H
 *B
 * H  B-A->H   AxB->H
 *
 * rev = true
 *            A
 *       F       H
 * F  AxB->F   B-A->F
 *B
 * H  A-B->F   A+B->HF
 *
 * F/H = Fill/Hole
 * A/B specify operands
 * + = union, - = subtraction, x = intersection
 * -> read as "produces"
 * FH = Fill surrounding holes
 * HF = Holes surrounding fill
 *
 * The operation of this function isn't very complicated.  It just traverses the crossings, and uses the crossing direction to decide whether the next segment should be from A or from B.
 */
Regions region_boolean(bool rev,
                       Region const & a, Region const & b,
                       Crossings const & cr_a, Crossings const & cr_b) {
    assert(cr_a.size() == cr_b.size());
    
    //If we are on the subtraction diagonal
    bool on_sub = logical_xor(a._fill, b._fill);
  
    Path ap = a.boundary(), bp = b.boundary();
    if(cr_a.empty()) {
        Regions ret;
        if(on_sub) {
            //is a subtraction
            if(logical_xor(a._fill, rev)) {
                //is B-A
                if(b.contains(ap.initialPoint())) {
                    ret.push_back(a);
                    ret.push_back(b);
                } else if(!a.contains(bp.initialPoint())) {
                    ret.push_back(b);
                }
            } else {
                //is A-B
                if(a.contains(bp.initialPoint())) {
                    ret.push_back(a);
                    ret.push_back(b);
                } else if(!b.contains(ap.initialPoint())) {
                    ret.push_back(a);
                }
            }
        } else if(logical_xor(a._fill, rev)) {
            //is A+B
            if(a.contains(bp.initialPoint())) ret.push_back(a); else
            if(b.contains(ap.initialPoint())) ret.push_back(b); else {
                ret.push_back(a);
                ret.push_back(b);
            }
        } else {
            //is AxB
            if(a.contains(bp.initialPoint())) ret.push_back(b); else
            if(b.contains(ap.initialPoint())) ret.push_back(a);
        }
        return ret;
    }

    //Traverse the crossings, creating path chunks:
    Regions chunks;
    std::vector<bool> visited_a(cr_a.size(), false), visited_b = visited_a;
    unsigned start_i = 0;
    while(true) {
        bool on_a = true;
        Path res;
        unsigned i = start_i;
        //this loop collects a single continuous Path (res) which is part of the result.
        do {
            Crossing prev;
            if(on_a) {
                prev = cr_a[i];
                visited_a[i] = true;
            } else {
                prev = cr_b[i];
                visited_b[i] = true; 
            }
            if(logical_xor(prev.dir, rev)) {
                if(on_a) i++; else i = find_crossing(cr_a, cr_b[i]);
                if(i >= cr_a.size()) i = 0;
                ap.appendPortionTo(res, prev.ta, cr_a[i].ta);
                on_a = true;
            } else {
                if(!on_a) i++; else i = find_crossing(cr_b, cr_a[i]);
                if(i >= cr_b.size()) i = 0;
                bp.appendPortionTo(res, prev.tb, cr_b[i].tb);
                on_a = false;
            }
        } while (on_a ? (!visited_a[i]) : (!visited_b[i]));
        
        //std::cout << rev << " c " << res.size() << "\n";

        chunks.push_back(Region(res, rev));
        
        std::vector<bool>::iterator unvisited = std::find(visited_a.begin(), visited_a.end(), false);
        if(unvisited == visited_a.end()) break; //visited all crossings
        start_i = unvisited - visited_a.begin();
    }
    
    //the fill of the container.  Only applies to FH/HF
    bool c_fill;
    if(!rev && (a._fill && b._fill)) c_fill = true; else
    if(rev && (!a._fill && !b._fill)) c_fill = false; else return chunks;
    if(chunks.size() > 1) {
        unsigned ix = outer_index(chunks);
        if(ix != chunks.size() && ix != 0) {
            Region temp = chunks[0];
            chunks[0] = chunks[ix];
            chunks[ix] = temp;
        }
    }
    if(chunks.size() > 0) chunks[0]._fill = c_fill;
    
    return chunks;
}

}
