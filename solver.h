#ifndef _SOLVE_SBASIS_H
#define _SOLVE_SBASIS_H
#include "point.h"
#include "s-basis.h"

unsigned
crossing_count(Geom::Point *V,	/*  Control pts of Bezier curve	*/
	       unsigned degree);	/*  Degree of Bezier curve 	*/
void
find_bezier_roots(
    Geom::Point *w, /* The control points  */
    unsigned degree,	/* The degree of the polynomial */
    std::vector<double> & solutions,	/* RETURN candidate t-values */
    unsigned depth);	/* The depth of the recursion */
#endif
