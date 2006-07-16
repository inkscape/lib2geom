#ifndef _SBASIS_TO_BEZIER
#define _SBASIS_TO_BEZIER

#include "multidim-sbasis.h"
// this produces a degree k bezier from a degree k sbasis
std::vector<double>
sbasis_to_bezier(SBasis const &B, unsigned q);

std::vector<Geom::Point>
sbasis_to_bezier(multidim_sbasis<2> const &B, unsigned q);

std::vector<Geom::Point>
sbasis2_to_bezier(multidim_sbasis<2> const &B, unsigned q);

#include "path-builder.h"
void
subpath_from_sbasis(Geom::PathBuilder &pb, multidim_sbasis<2> const &B, double tol);
void
subpath_from_sbasis_incremental(Geom::PathBuilder &pb, multidim_sbasis<2> B, double tol);

#endif
