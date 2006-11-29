#include "matrix.h"

namespace Geom {

Matrix operator*(translate const &t, scale const &s);
Matrix operator*(translate const &t, rotate const &r);
Matrix operator*(translate const &t, Matrix const &m);

Matrix operator*(scale const &s, translate const &t);
Matrix operator*(scale const &s, Matrix const &m);

Matrix operator*(rotate const &a, Matrix const &b);

Matrix operator*(Matrix const &m, translate const &t);
Matrix operator/(Matrix const &m, scale const &s);
Matrix operator*(Matrix const &m, scale const &s);
Matrix operator*(Matrix const &m, rotate const &r);

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
