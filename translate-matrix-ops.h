#ifndef SEEN_LIBGeom_Geom_TRANSLATE_MATRIX_OPS_H
#define SEEN_LIBGeom_Geom_TRANSLATE_MATRIX_OPS_H

#include "translate.h"
#include "matrix.h"

namespace Geom {
Matrix operator*(translate const &t, Matrix const &m);
}


#endif /* !SEEN_LIBGeom_Geom_TRANSLATE_MATRIX_OPS_H */

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
