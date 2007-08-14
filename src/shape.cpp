#include "shape.h"

#include <iostream>
#include <algorithm>

namespace Geom {

bool logical_xor (bool a, bool b) { return (a || b) && !(a && b); }

Region Region::operator*(Matrix const &m) const {
    return Region(_boundary * m, logical_xor(m.flips(), _fill));
}

Shape Shape::operator*(Matrix const &m) const {
    Regions hs;
    for(Regions::const_iterator i = inners.begin(); i != inners.end(); i++)
        hs.push_back((*i) * m);
    return Shape(outer * m, hs);
}

bool disjoint(Path const & a, Path const & b) {
    return !contains(a, b.initialPoint()) && !contains(b, a.initialPoint());
}

struct SweepObject {
    unsigned ix;
    bool on_a;
    std::vector<unsigned> intersects;
    
    SweepObject(unsigned i, bool a) : ix(i), on_a(a) {}
};

struct Event {
    double x;
    SweepObject *val;
    bool closing;
    
    friend std::vector<SweepObject> region_pairs(std::vector<Event> const & es);
    
    Event(double t, SweepObject *v, bool c) : x(t), val(v), closing(c) {}
    
    bool operator<(Event const &other) {
        if(x < other.x) return true;
        if(x > other.x) return false;
        return closing < other.closing;
    }
};

typedef std::vector<Event> Events;

std::vector<SweepObject> sweep(Events const & es) {
    std::vector<SweepObject> returns;
    
    std::vector<SweepObject*> open[2];
    for(Events::const_iterator e = es.begin(); e != es.end(); ++e) {
        unsigned ix = e->val->on_a ? 0 : 1;
        if(e->closing) {
            for(int i = open[ix].size()-1; i >= 0; --i) {
                if(open[ix][i] == e->val) {
                    open[ix].erase(open[ix].begin() + i);
                    break;
                }
            }
        } else {
            open[ix].push_back(e->val);
        }
        if(e->val->on_a) {
            if(e->closing) {
                SweepObject *p = e->val;
                returns.push_back(*p);
                delete p;
            } else {
                for(unsigned i = 0; i < open[1].size(); i++) {
                    e->val->intersects.push_back(open[1][i]->ix);
                }
            }
        } else {
            if(!e->closing) {
                for(unsigned i = 0; i < open[0].size(); i++) {
                    open[0][i]->intersects.push_back(e->val->ix);
                }
            }
        }
    }
    
    return returns;
}

template <typename T>
Events events(Regions const & a, Regions const & b) {
    Events ret;
    for(unsigned i = 0; i < a.size(); i++) {
        Rect bounds = a[i].boundsFast();
        SweepObject *obj = new SweepObject(i, true);
        ret.push_back(Event(bounds.left, obj, false));
        ret.push_back(Event(bounds.right, obj, true));
    }
    for(unsigned i = 0; i < b.size(); i++) {
        Rect bounds = b[i].boundsFast();
        SweepObject *obj = new SweepObject(i, false);
        ret.push_back(Event(bounds.left, obj, false));
        ret.push_back(Event(bounds.right, obj, true));
    }
    std::sort(ret.begin(), ret.end());
    return ret;
}

// inverse is like a boolean not.
Shape Shape::inverse() const {
    Shape ret(outer.inverse());
    for(unsigned i = 0; i < inners.size(); i++) {
        ret.inners.push_back(inners[i].inverse());
    }
    return ret;
}

/*
Shape shape_region_boolean(bool rev, Shape const & a, Region const & b) {
    Shape ret;
    
    Path pb = b.boundary();
    
    for(Regions::const_iterator i = a.content.begin(); i != a.content.end(); ++i) {
        Crossings cr = crossings(i->boundary(), pb);
        if(!cr.empty()) {
            ret = path_boolean(rev, *i, b, cr);
        }
    }
    return ret;
}
*/

std::vector<SweepObject> fake_cull(Regions const &a, Regions const &b) {
    std::vector<SweepObject> ret;
    std::vector<unsigned> all;
    for(unsigned j = 0; j < b.size(); j++) {
        all.push_back(j);
    }
    
    for(unsigned i = 0; i < a.size(); i++) {
        SweepObject res(i, true);
        res.intersects = all;
        ret.push_back(res);
    }
    
    return ret;
}

/*
void union_region(Shape &a, Region const & b) {
    if(b.fill()) {
        for(unsigned i = 0; i < a.inners.size(); i++) {
            Shape res = path_boolean(false, a.inners[i], b)
        }
    }
}

//a - (x+y)
Shape subtract_merge(Regions const & a, Regions const & x, Regions const & y) {
    
}*/



Shapes shape_intersect(Shape const & a, Shape const & b) {
    Regions oint = region_boolean(true, a.outer, b.outer);

    std::vector<SweepObject> es = fake_cull(a.inners, b.inners);
    
    std::vector<bool> used(false, b.inners.size());
    Regions acc;
    //TODO: this thing doesn't work if A has inners filled and B has inners holed, as then B's type isn't equal to the output type.
    //TODO: one possible optimization might be to use the bbox crossing list to narrow down possible acc matches
    for(std::vector<SweepObject>::iterator i = es.begin(); i != es.end(); i++) {        
        Regions changed;         //new or changed this iteration
        Regions leftovers;       //not changed this iteration
        changed.push_back(a.inners[i->ix]);
        
        //add this inner to already accumulated inners
        for(unsigned j = 0; j < acc.size(); j++) {
            bool used_acc = false;
            Regions new_changed;
            for(unsigned k = 0; k < changed.size(); k++) {
                Crossings cr = crossings(changed[k].boundary(), acc[j].boundary());
                if(!cr.empty()) {
                    Regions res = region_boolean(true, changed[k], acc[j], cr);
                    new_changed.insert(new_changed.end(), res.begin(), res.end());
                    used_acc = true;
                } else {
                    new_changed.push_back(changed[k]);
                }
            }
            if(used_acc) changed = new_changed; else leftovers.push_back(acc[j]);
        }
        
        //Add b inners which are so-far unused.
        //This is similar to the above loop, though must be done seperately to use the cull data.
        for(std::vector<unsigned>::iterator j = i->intersects.begin(); j != i->intersects.end(); j++) {
            if(!used[*j]) {
                Regions new_changed;
                for(unsigned k = 0; k < changed.size(); k++) {
                    Crossings cr = crossings(changed[k].boundary(), b.inners[*j].boundary());
                    if(!cr.empty()) {
                        Regions res = region_boolean(true, changed[k], b.inners[*j], cr);
                        new_changed.insert(new_changed.end(), res.begin(), res.end());
                        used[*j] = true;
                        continue;
                    }
                    new_changed.push_back(changed[k]);
                }
                changed = new_changed;
            }
        }
        leftovers.insert(leftovers.end(), changed.begin(), changed.end());
        acc = leftovers;
    }
    
    // subtract/distribute the holes
    es = fake_cull(oint, acc);
    Shapes results;
    for(std::vector<SweepObject>::iterator i = es.begin(); i != es.end(); i++) {
        Regions repl;
        bool used;
        for(std::vector<unsigned>::iterator j = i->intersects.begin(); j != i->intersects.end(); j++) {
            Crossings cr = crossings(oint[i->ix].boundary(), acc[*j].boundary());
            if(!cr.empty()) {
                repl = region_boolean(true, oint[i->ix], acc[*j], cr);
                
            }// else if() {
            //}
        }
    }
}

Shapes shape_subtract(Shape const & a, Shape const & b) {
    return shape_intersect(a, b.inverse());
}

//union = (Ao + Bo) - (Ah + Bh)
/*Shape shape_union(Shape const & a, Shape const & b) {
    std::vector<SweepObject> es = fake_cull(a.content, b.content); //sweep(events(a.fills, b.fills));
    
    Regions const &ac = a.content, bc = b.content;
    
    Shape ret;
    for(unsigned i = 0; i < es.size(); i++) {
        SweepObject cur = es[i];
        std::vector<unsigned> ints = cur.intersects;
        if(ints.size() > 0) {
            Shape acc(a.content[cur.ix]);
            for(unsigned j = 0; j < es[i].intersects.size(); j++) {
            
                if(es[i].fill == b.content[es[i].intersects[j]].fill
                acc = shape_region_boolean(false, acc, b.content[es[i].intersects[j]]);
            }
            ret.mergeWith(acc);
        }
    }
    
    return ret;
}*/

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
            if(contains(ps[i].boundary(), exemplar)) {
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
        
        std::cout << rev << " c " << res.size() << "\n";

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
            std::cout << "cfill " << ix << " with " << c_fill << "\n";
        if(ix != chunks.size()) {
            Region temp = chunks[0];
            chunks[ix]._fill = c_fill;
            chunks[0] = chunks[ix];
            chunks[ix] = temp;
        }
    } else if(chunks.size() == 1) {
        chunks[0]._fill = c_fill;
    }
    
    return chunks;
}


/*    

    bool on_holes = 
    for(Paths::iterator i = a.fills.begin(); ; ++i) {
        if(i == a.fills.end()) i = a;
        if(i == a.holes.end()) break;
        for(Paths::iterator j = b.fills.begin(); j != b.fills.end(); ++j) {
            Crossings cr = crossings(*i, *j);
            if(!cr.empty) {
                path_boolean(false, *i, *j, cr);
            } else if(contains(*i, j->initialPoint())) {
                
            } 
        }
    }
*/
/*
Shape regions_boolean(bool rev, Regions const &a, Regions const &b) {
    std::vector<SweepObject> es = fake_cull(a, b);
    
    Shape ret;
    for(std::vector<SweepObject>::iterator i = es.begin(); i != es.end(); i++) {
        for(std::vector<unsigned>::iterator j = i->intersects.begin(); j != i->intersects.end(); j++) {
            Shape res = path_boolean(rev, a[i->ix], b[*j]);
            
}

Shape shape_union(Shape const & a, Shape const & b) {
    Regions afill = a.fill, bfill = b.fill;
    
    std::vector<SweepObject> es = fake_cull(afill, bfill);
    
    for(std::vector<SweepObject>::iterator i = es.begin(); i != es.end(); i++) {
        for(std::vector<unsigned>::iterator j = i->intersects.begin(); j != i->intersects.end(); j++) {
            Region ar = ac[i->ix], br = bfill[*j];
            if(ar.fill == br.fill) {
                ac[i->ix] = path_boolean(false, ar, br);
            }
        }
    }
}
*/
/*
    //Get sorted sets of crossings
    Crossings cr = crossings(a.outer, b.outer);
    
    if(cr.empty() && disjoint(a.outer, b.outer)) {
        Shape ret(a);
        ret.fills.insert(b.
        ret.holes.
        return returns;
    }
    
    Crossings cr_a = cr, cr_b = cr;
    sortA(cr_a); sortB(cr_b);
    
    Shape ret = path_union(a.outer, b.outer, cr_a_copy, cr_b_copy).front();
    //Copies of the holes, so that some may be removed / replaced by portions
    Paths holes[] = { a.holes, b.holes };
    
    // iterate the intersections of the paths, and deal with the holes within
    Paths inters = path_intersect(a.outer, b.outer, cr_a, cr_b);
    for(Paths::iterator inter = inters.begin(); inter != inters.end(); inter++) {
        Paths withins[2];  //These are the portions of holes that are inside the intersection
        
        //Take holes from both operands and 
        for(unsigned p = 0; p < 2; p++) {
            for (Replacer<Paths> holei(&holes[p]); !holei.ended(); ++holei) {
                Crossings hcr = crossings(*inter, *holei);
                if(!hcr.empty()) {
                    Crossings hcr_a = hcr, hcr_b = hcr;
                    sortA(hcr_a); sortB(hcr_b);
                    Paths innards = path_intersect_reverse(*inter, *holei, hcr);
                    if(!innards.empty()) {
                        //stash the stuff which is inside the intersection
                        withins[p].insert(withins[p].end(), innards.begin(), innards.end());
                        
                        //replaces the original holes entry with the remaining fragments
                        Paths remains = shapes_to_paths<Shapes>(path_subtract_reverse(*inter, *holei, hcr_a, hcr_b));
                        holei.replace(remains);
                    }
                } else if(contains(*inter, holei->initialPoint())) {
                    withins[p].push_back(*holei);
                    holei.erase();
                }
            }
        }
        
        for(Paths::iterator j = withins[0].begin(); j!= withins[0].end(); j++) {
            for(Paths::iterator k = withins[1].begin(); k!= withins[1].end(); k++) {
                Crossings hcr = crossings(*j, *k);
                //TODO: use crosses predicate
                if(!hcr.empty()) {
                    //By the nature of intersect, we don't need to accumulate
                    Paths ps = path_intersect(*j, *k, hcr);
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
*/
/*
void reverse_crossings_direction(Crossings &cr) {
    for(unsigned i = 0; i < cr.size(); i++) {
        cr[i].dir = !cr[i].dir;
    }
}

Shapes shape_subtract(Shape const & ac, Shape const & b) {
    //TODO: use crosses predicate
    Crossings cr = crossings(ac.outer, b.outer);
    Shapes returns;
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
            returns.push_back(ac);
            return returns;
        }
    }
    Shape a = ac;
    
    //subtractor accumulator
    Shape sub = b;

    //First, we deal with the outer-path - add intersecting holes in a to it 
    Paths remains;  //holes which intersected - needed later to remove from islands (holes in subtractor)
    for(Eraser<Paths> i(&a.holes); !i.ended(); ++i) {std::cout << "eins\n";
        Crossings hcr = crossings(sub.outer, *i);
        //TODO: use crosses predicate
        if(!hcr.empty()) {
            //Paths old_holes = sub.holes;
            sub = path_union_reverse(sub.outer, *i).front();
            
            remains.push_back(*i);
            i.erase();
        } else if(contains(sub.outer, i->initialPoint())) {
            remains.push_back(*i);
            i.erase();
        }
    }

    //Next, intersect the subtractor holes with a's outer path, and subtract a's holes from the result
    //This yields the 'islands'
    for(Paths::iterator i = sub.holes.begin(); i != sub.holes.end(); i++) {
        std::cout << "\n*";
        Shapes new_islands = path_boolean(INTERSECT, a.outer, i->reverse());
        for(Paths::iterator hole = remains.begin(); hole != remains.end(); hole++) {  //iterate a's holes that are inside/intersected by the subtractor
            std::cout << "\n *";
            for(Replacer<Shapes> isle(&new_islands); !isle.ended(); ++isle) { // iterate the islands
                std::cout << "\n  *";
                //since the holes are disjoint, we don't need to do a recursive shape_subtract
                Crossings hcr = crossings(isle->outer, *hole);
                //TODO: use crosses predicate
                if(!hcr.empty()) {
                    Shapes split = path_subtract(isle->outer, *hole, hcr);
                    add_holes(split, isle->holes);
                    isle.replace(split);
                } else if(contains(isle->outer, hole->initialPoint())) {
                    Shape x = *isle;
                    x.holes.push_back(*hole);
                    isle.replace(x);
                } else if(contains(*hole, isle->outer.initialPoint())) {
                    isle.erase();
                }
            }
        }
        returns.insert(returns.end(), new_islands.begin(), new_islands.end());
    }
    
    if(flag_inside) {
        a.holes.push_back(sub.outer.reverse());
        returns.push_back(a);
    } else {
        Shapes outers = path_subtract(a.outer, sub.outer);
        add_holes(outers, a.holes);
        
        returns.insert(returns.end(), outers.begin(), outers.end());
    }
    
    return returns;
}

Shapes shape_intersect(Shape const & a, Shape const & b) {
    Shapes ret;

    //Get sorted sets of crossings
    Crossings cr = crossings(a.outer, b.outer);
    if(cr.empty() && disjoint(a.outer, b.outer)) { return ret; }
    
    ret = path_boolean(INTERSECT, a.outer, b.outer, cr);
    
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

Shape path_boolean_reverse(bool btype, Region const & a, Region const & b, Crossings const &cr) {
    Crossings new_cr;
    Path bp = b.boundary();
    double max = bp.size();
    for(Crossings::const_iterator i = cr.begin(); i != cr.end(); i++) {
        Crossing x = *i;
        if(x.tb > max) x.tb = 1 - (x.tb - max) + max; // on the last seg - flip it about
        else x.tb = max - x.tb;
        x.dir = !x.dir;
        new_cr.push_back(x);
    }
    return path_boolean(btype, a, bp.reverse(), new_cr);
}
*/

}
