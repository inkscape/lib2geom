#include "region.h"
#include "utils.h"

namespace Geom {

Region Region::operator*(Matrix const &m) const {
    return Region(boundary * m, logical_xor(m.flips(), fill));
}

bool Region::invariants() const {
    return self_crossings(boundary).empty();
}

/*
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
*/

unsigned outer_index(Regions const &ps) {
    if(ps.size() <= 1 || contains(ps[0].getBoundary(), ps[1].getBoundary().initialPoint())) {
        return 0;
    } else {
        /* Since we've already shown that chunks[0] is not outside
           it can be used as an exemplar inner. */
        Point exemplar = ps[0].getBoundary().initialPoint();
        for(unsigned i = 1; i < ps.size(); i++) {
            if(ps[i].contains(exemplar)) {
                std::cout << "oi: " << i << "\n";
                return i;
            }
        }
    }
    return ps.size();
}

}
