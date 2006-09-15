#include "matrix-translate-ops.h"
#include "rotate-ops.h"

Geom::Matrix
operator*(Geom::translate const &a, Geom::rotate const &b)
{
    return Geom::Matrix(b) * Geom::translate(a.offset * b);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
