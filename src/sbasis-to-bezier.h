#ifndef _SBASIS_TO_BEZIER
#define _SBASIS_TO_BEZIER

#include "multidim-sbasis.h"
#include "path-builder.h"
#include "path2.h"

namespace Geom{
// this produces a degree k bezier from a degree k sbasis
std::vector<double>
sbasis_to_bezier(SBasis const &B, unsigned q = 0);

std::vector<Geom::Point>
sbasis_to_bezier(MultidimSBasis<2> const &B, unsigned q);

std::vector<Geom::Point>
sbasis2_to_bezier(MultidimSBasis<2> const &B, unsigned q);

void
subpath_from_sbasis(Geom::PathSetBuilder &pb, MultidimSBasis<2> const &B, double tol, bool initial=true);
void
subpath_from_sbasis_incremental(Geom::PathSetBuilder &pb, MultidimSBasis<2> B, double tol, bool initial=true);

void
path_from_sbasis(Geom::Path2::Path &pb, MultidimSBasis<2> const &B, double tol);

};
#endif
