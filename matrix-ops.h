/* operator functions for Geom::Matrix. */
#ifndef SEEN_Geom_MATRIX_OPS_H
#define SEEN_Geom_MATRIX_OPS_H

#include "matrix.h"

namespace Geom {

inline bool operator==(Matrix const &a, Matrix const &b)
{
    for(unsigned i = 0; i < 6; ++i) {
        if ( a[i] != b[i] ) {
            return false;
        }
    }
    return true;
}

inline bool operator!=(Matrix const &a, Matrix const &b)
{
    return !( a == b );
}

Matrix operator*(Matrix const &a, Matrix const &b);

} /* namespace Geom */


#endif /* !SEEN_Geom_MATRIX_OPS_H */

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
