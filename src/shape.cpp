#include "shape.h"
#include "utils.h"
#include "sweep.h"

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

// inverse is a boolean not.
Shape Shape::inverse() const {
    Shape ret(outer.inverse());
    for(unsigned i = 0; i < inners.size(); i++) {
        ret.inners.push_back(inners[i].inverse());
    }
    return ret;
}

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
    std::vector<std::vector<unsigned> > es = fake_cull(a.inners.size(), b.inners.size());
    for(unsigned i = 0; i < a.inners.size(); i++)
        for(unsigned j = 0; j < es[i].size(); j++)
            append(holes, region_boolean(false, a.inners[i], b.inners[es[i][j]]));
    
    ret.push_back(Shape(outers.front(), holes));
    return ret;
}

//outers - inners
Shapes do_holes(Regions const & outers, Regions const & inners) {
    //classifies the holes
    Regions extra_fills, crossers, non_crossers;
    
    //info on crossers:
    std::vector<Crossings> crs;
    std::vector<std::vector<unsigned> > ixs(outers.size(), std::vector<unsigned>()); //relates outer indices to inner intersectors
    
    for(unsigned i = 0; i < inners.size(); i++) {
        if(inners[i].fill()) {
            extra_fills.push_back(inners[i]);
            continue;
        }
        bool crossed = false;
        for(unsigned j = 0; j < outers.size(); j++) {
            Crossings cr = crossings(outers[j].boundary(), inners[i].boundary());
            if(!cr.empty()) {
                crossers.push_back(inners[i]);
                crs.push_back(cr);
                ixs[j].push_back(crossers.size() - 1);
                crossed = true;
            }
        }
        if(!crossed) non_crossers.push_back(inners[i]);
    }
    
    Regions result_outers;

    //subtract the crossers
    for(unsigned i = 0; i < outers.size(); i++) {
        if(ixs[i].size() > 1) {
            Regions repl;
            append(repl, region_boolean(true, outers[i], crossers[ixs[i][0]], crs[ixs[i][0]]));
            for(unsigned j = 1; j < ixs[i].size(); j++) {
                Regions new_repl;
                for(unsigned k = 0; k < repl.size(); k++) {
                    append(new_repl, region_boolean(true, repl[k], crossers[ixs[i][j]]));
                }
                repl = new_repl;
            }
            append(result_outers, repl);
        } else if(ixs[i].size() == 1) {
            append(result_outers, region_boolean(true, outers[i], crossers[ixs[i][0]], crs[ixs[i][0]]));
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

//R = (Ao - Bo) - (Ao x Bi)
Shapes shape_subtract(Shape const & a, Shape const & b) {
    Shape br = b.inverse();
    
    //Ao - Bo
    Regions outers = region_boolean(true, a.outer, br.outer);
    
    //Ao x Bi
    for(unsigned i = 0; i < br.inners.size(); i++) {
        Regions res = region_boolean(true, a.outer, br.inners[i]);
        outers.insert(outers.end(), res.begin(), res.end());
    }
    
    //- Ai
    return do_holes(outers, a.inners);
}

Regions merge_holes(Regions const & a, Regions const & b) {
    Shapes inners = do_holes(a, b);
    Regions as_regions;
    for(unsigned i = 0; i < inners.size(); i++) {
        as_regions.push_back(inners[i].getOuter());
    }
}

//R = (Ao x Bo) - (Ai + Bi)
Shapes shape_intersect(Shape const & a, Shape const & b) {
    //Ao x Bo
    Regions outers = region_boolean(true, a.outer, b.outer);

    //- (Ai + Bi)
    return do_holes(outers, merge_holes(a.inners, b.inners));
}

//R = (A-B) ++ (B-A) or R = (A+B) - (AxB)
Shapes shape_exclude(Shape const & a, Shape const & b) {
    Shapes results = shape_subtract(a, b);
    append(results, shape_subtract(b, a));
    return results;
}

Shapes sanitize_path(Path const &ps) {
    Crossings cr = self_crossings(ps);
    
}

Shapes sanitize_paths(std::vector<Path> const &ps) {
    
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

}
