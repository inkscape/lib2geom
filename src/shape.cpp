#include "shape.h"
#include "utils.h"

#include <iostream>
#include <algorithm>

namespace Geom {

bool logical_xor (bool a, bool b) { return (a || b) && !(a && b); }

bool disjoint(Path const & a, Path const & b) {
    return !contains(a, b.initialPoint()) && !contains(b, a.initialPoint());
}

template<typename T>
void append(T &a, T const &b) {
    a.insert(a.end(), b.begin(), b.end());
}

Shape Shape::operator*(Matrix const &m) const {
    Regions hs;
    for(Regions::const_iterator i = inners.begin(); i != inners.end(); i++)
        hs.push_back((*i) * m);
    return Shape(outer * m, hs);
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

// Ro = Ao + Bo
// Rh = (Ai - Bo) ++ (Bi - Ao) ++ (Ai x Bi)
Shapes shape_union(Shape const & a, Shape const & b) {
    Shapes ret;
    
    //Ao + Ab
    Crossings cr = crossings(a.outer.boundary(), b.outer.boundary());
    Regions outers;
    if(cr.empty()) {
        if(a.outer.contains(b.outer.boundary().initialPoint())) {
            outers.push_back(a.outer);
        } else if(b.outer.contains(a.outer.boundary().initialPoint())) {
            outers.push_back(b.outer);
        } else {
            ret.push_back(a); ret.push_back(b);
            return ret;
        }
    } else {
        outers = region_boolean(false, a.outer, b.outer, cr);
    }
    
    Regions holes;
    
    for(unsigned i = 1; i < outers.size(); i++)
        holes.push_back(outers[i]);
    
    //Ai - Bo
    for(unsigned i = 0; i < a.inners.size(); i++)
        append(holes, region_boolean(false, a.inners[i], b.outer));
    //Bi - Ao
    for(unsigned i = 0; i < b.inners.size(); i++)
        append(holes, region_boolean(false, b.inners[i], a.outer));
    
    //Ai x Bi
    std::vector<SweepObject> es = fake_cull(a.inners, b.inners);
    for(std::vector<SweepObject>::iterator i = es.begin(); i != es.end(); i++)
        for(std::vector<unsigned>::iterator j = i->intersects.begin(); j != i->intersects.end(); j++)
            append(holes, region_boolean(false, a.inners[i->ix], b.inners[*j]));
    
    ret.push_back(Shape(outers.front(), holes));
    return ret;
}

void add_holes(Shapes &x, Regions const &h) {
    for(Regions::const_iterator j = h.begin(); j != h.end(); j++) {
        for(Shapes::iterator i = x.begin(); i != x.end(); i++) {
            if(i->outer.contains(j->boundary().initialPoint())) {
                i->inners.push_back(*j);
                break;
            }
        }
    }
}

//outers - inners
Shapes do_holes(Regions const & outers, Regions const & inners) {
    //classifies the holes
    Regions extra_fills, crossers, non_crossers;
    
    //info on crossers:
    std::vector<Crossings> crs;
    std::vector<std::vector<unsigned> > ixs; //relates outer indices to inner intersectors
    for(unsigned j = 0; j < outers.size(); j++) ixs.push_back(std::vector<unsigned>());
    
    unsigned crix = 0;
    for(unsigned i = 0; i < inners.size(); i++) {
        if(inners[i].fill()) {
            extra_fills.push_back(inners[i]);
            continue;
        }
        for(unsigned j = 0; j < outers.size(); j++) {
            Crossings cr = crossings(outers[j].boundary(), inners[i].boundary());
            if(!cr.empty()) {
                crossers.push_back(inners[i]);
                crs.push_back(cr);
                ixs[j].push_back(crix);
                crix++;
                goto skip;
            } 
        }
        non_crossers.push_back(inners[i]);
        skip:
        (void)0;
    }
    
    std::cout << extra_fills.size() << "n";
    
    
    Regions result_outers;

    //subtract the crossers
    for(unsigned i = 0; i < outers.size(); i++) {
        if(ixs[i].size() > 1) {
            Regions repl;
            append(repl, region_boolean(true, outers[i], crossers[ixs[i][0]], crs[ixs[i][0]]));
            for(unsigned jp = 1; jp < ixs[i].size(); jp++) {
                unsigned j = ixs[i][jp];
                Regions new_repl;
                for(unsigned k = 0; k < repl.size(); k++) {
                    append(new_repl, region_boolean(true, repl[k], crossers[j]));
                }
                repl = new_repl;
            }
            append(result_outers, repl);
        } else {
            result_outers.push_back(outers[i]);
        }
    }

    Shapes results = shapes_from_regions(result_outers);
    
    //distribute the non-crossers
    for(unsigned j = 0; j < non_crossers.size(); j++) {
        for(Shapes::iterator i = results.begin(); i != results.end(); i++) {
            if(i->outer.contains(non_crossers[j].boundary().initialPoint())) {
                i->inners.push_back(non_crossers[j]);
                break;
            }
        }
    }
    
    //add the extra fills
    for(unsigned i = 0; i < extra_fills.size(); i++) {
        results.push_back(Shape(extra_fills[i]));
    }
    
    return results;
}

Shapes shape_subtract(Shape const & a, Shape const & b) {
    Shape br = b.inverse();
    //Ao - Bo
    Regions outers = region_boolean(true, a.outer, br.outer);
    
    //Ao x Bi
    for(unsigned i = 0; i < br.inners.size(); i++) {
        Regions res = region_boolean(true, a.outer, br.inners[i]);
        outers.insert(outers.end(), res.begin(), res.end());
    }
    
    //outers - Ai
    return do_holes(outers, a.inners);
}

Shapes shape_intersect(Shape const & a, Shape const & b) {
    Regions outers = region_boolean(true, a.outer, b.outer);
    
    //Ah + Bh
    std::vector<bool> used(false, b.inners.size());
    Regions acc;
    std::vector<SweepObject> es = fake_cull(a.inners, b.inners);
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
    
    //outers - subs
    //outers.insert(outers.end(), acc.begin(), acc.end());
    return do_holes(outers, acc); //shapes_from_regions(outers);
}

Shapes shape_exclude(Shape const & a, Shape const & b) {
    Shapes results = shape_subtract(a, b);
    append(results, shape_subtract(b, a));
    return results;
}

/*
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
*/

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
} 

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

void reverse_crossings_direction(Crossings &cr) {
    for(unsigned i = 0; i < cr.size(); i++) {
        cr[i].dir = !cr[i].dir;
    }
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
