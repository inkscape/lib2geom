#include "libnr/matrix-ops.h"

Geom::Matrix
operator/(Geom::Matrix const &m, Geom::scale const &s)
{
    using Geom::X; using Geom::Y;
    Geom::Matrix ret(m);
    ret[0] /= s[X]; ret[1] /= s[Y];
    ret[2] /= s[X]; ret[3] /= s[Y];
    ret[4] /= s[X]; ret[5] /= s[Y];
    assert_close( ret, m * Geom::Matrix(s.inverse()) );
    return ret;
}

Geom::Matrix
operator*(Geom::Matrix const &m, Geom::scale const &s)
{
    using Geom::X; using Geom::Y;
    Geom::Matrix ret(m);
    ret[0] *= s[X]; ret[1] *= s[Y];
    ret[2] *= s[X]; ret[3] *= s[Y];
    ret[4] *= s[X]; ret[5] *= s[Y];
    assert_close( ret, m * Geom::Matrix(s) );
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
