#ifndef SEEN_LIBGeom_Geom_MATRIX_TRANSLATE_OPS_H
#define SEEN_LIBGeom_Geom_MATRIX_TRANSLATE_OPS_H

/** \file 
 * Declarations (and definition if inline) of operator 
 * blah (Geom::Matrix, Geom::translate).
 */

#include "matrix.h"
#include "translate.h"

//Geom::Matrix operator*(Geom::Matrix const &m, Geom::translate const &t);

namespace Geom {
Matrix operator*(Matrix const &m, translate const &t);
}

inline Geom::Matrix
operator/(Geom::Matrix const &numer, Geom::translate const &denom)
{
    return numer * Geom::translate(-denom.offset);
}


#endif /* !SEEN_LIBGeom_Geom_MATRIX_TRANSLATE_OPS_H */

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
