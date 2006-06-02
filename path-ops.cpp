#include "path-ops.h"

namespace Geom{

double curvature(SubPath const& p, SubPath::SubPathLocation const& pl) {
    Geom::Point pos, tgt, acc;
    p.point_tangent_acc_at(pl, pos, tgt, acc);
    double kurvature = dot(acc, rot90(tgt))/pow(Geom::L2(tgt),3);
    
    Geom::Point kurv_vector = (1./kurvature)*Geom::unit_vector(rot90(tgt));
}

};
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
