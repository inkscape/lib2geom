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
    sortA(cr_a); sortB(cr_b);
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

unsigned find_crossing(Crossings const &cr, Crossing x) {
    return std::find(cr.begin(), cr.end(), x) - cr.begin();
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
Regions region_boolean( bool rev,
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
