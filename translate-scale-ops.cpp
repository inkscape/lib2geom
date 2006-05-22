#include "matrix-ops.h"

Geom::Matrix
operator*(Geom::translate const &t, Geom::scale const &s)
{
    using Geom::X; using Geom::Y;

    Geom::Matrix ret(s);
    ret[4] = t[X] * s[X];
    ret[5] = t[Y] * s[Y];
    assert_close( ret, Geom::Matrix(t) * Geom::Matrix(s) );
    return ret;
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
