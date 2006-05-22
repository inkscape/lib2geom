#include "libnr/matrix-ops.h"
#include "libnr/point-matrix-ops.h"

Geom::Point operator/(Geom::Point const &p, Geom::Matrix const &m) {
    return p * m.inverse();
}

Geom::Matrix operator/(Geom::Matrix const &a, Geom::Matrix const &b) {
    return a * b.inverse();
}


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
