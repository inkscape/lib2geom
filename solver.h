#ifndef _SOLVE_SBASIS_H
#define _SOLVE_SBASIS_H
#include "point.h"
#include "s-basis.h"

int
crossing_count(Geom::Point *V,	/*  Control pts of Bezier curve	*/
	       int degree);	/*  Degree of Bezier curve 	*/
int
find_bezier_roots(
    Geom::Point *w, /* The control points  */
    int degree,	/* The degree of the polynomial */
    double *t,	/* RETURN candidate t-values */
    int depth);	/* The depth of the recursion */
#endif
