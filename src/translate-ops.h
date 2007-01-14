#ifndef SEEN_Geom_TRANSLATE_OPS_H
#define SEEN_Geom_TRANSLATE_OPS_H

#include "translate.h"
#include "point-ops.h"

namespace Geom {

inline bool operator==(Translate const &a, Translate const &b) {
    return a.offset == b.offset;
}

inline bool operator!=(Translate const &a, Translate const &b) {
    return !( a == b );
}

inline Point operator*(Point const &v, Translate const &t) {
    return t.offset + v;
}

inline Translate operator*(Translate const &a, Translate const &b) {
    return Translate( a.offset + b.offset );
}

} /* namespace Geom */


#endif /* !SEEN_Geom_TRANSLATE_OPS_H */

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
