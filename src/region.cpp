#include "region.h"
#include "utils.h"

#include "shape.h"

namespace Geom {

Region Region::operator*(Matrix const &m) const {
    return Region((m.flips() ? boundary.reverse() : boundary) * m, fill);
}

bool Region::invariants() const {
    return self_crossings(boundary).empty();
}

Regions path_union(Region const & a, Region const & b) {
    return shape_boolean(false, Shape(a.asFill()), Shape(b.asFill())).getContent();
}

Regions path_subtract(Region const & a, Region const & b) {
    return shape_boolean(true, Shape(a.asFill()), Shape(b.asHole())).getContent();
}

Regions path_intersect(Region const & a, Region const & b) {
    return shape_boolean(true, Shape(a.asFill()), Shape(b.asFill())).getContent();
}

Regions path_exclude(Region const & a, Region const & b) {
    Regions ret = path_subtract(a, b);
    Regions add = path_subtract(b, a);
    ret.insert(ret.end(), add.begin(), add.end());
    return ret;
}

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
