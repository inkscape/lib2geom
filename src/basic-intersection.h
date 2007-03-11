#include "s-basis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "multidim-sbasis-bounds.h"

namespace Geom {

std::vector<std::pair<double, double> > 
find_intersections( Geom::MultidimSBasis<2> const & A, 
                    Geom::MultidimSBasis<2> const & B);

std::vector<std::pair<double, double> > 
find_self_intersections(Geom::MultidimSBasis<2> const & A);

// Bezier form
std::vector<std::pair<double, double> > 
find_intersections( vector<Geom::Point> const & A, 
                    vector<Geom::Point> const & B);

std::vector<std::pair<double, double> > 
find_self_intersections(vector<Geom::Point> const & A);

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
