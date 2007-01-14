#include "matrix.h"

namespace Geom {
Matrix operator*(Matrix const &m0, Matrix const &m1);
    
Geom::Point operator/(Geom::Point const &, Geom::Matrix const &);
Geom::Matrix operator/(Geom::Matrix const &, Geom::Matrix const &);

Matrix operator*(Translate const &t, Scale const &s);
Matrix operator*(Translate const &t, Rotate const &r);
Matrix operator*(Translate const &t, Matrix const &m);

Matrix operator*(Scale const &s, Translate const &t);
Matrix operator*(Scale const &s, Matrix const &m);

Matrix operator*(Rotate const &a, Matrix const &b);

Matrix operator*(Matrix const &m, Translate const &t);
Matrix operator/(Matrix const &m, Scale const &s);
Matrix operator*(Matrix const &m, Scale const &s);
Matrix operator*(Matrix const &m, Rotate const &r);

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
