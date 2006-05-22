#ifndef SEEN_LIBGeom_Geom_TRANSLATE_SCALE_OPS_H
#define SEEN_LIBGeom_Geom_TRANSLATE_SCALE_OPS_H

#include "matrix.h"
#include "translate.h"
#include "scale.h"

Geom::Matrix operator*(Geom::translate const &t, Geom::scale const &s);


#endif /* !SEEN_LIBGeom_Geom_TRANSLATE_SCALE_OPS_H */

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
