#define __Geom_PATH_C__

#include "path.h"

static void
nr_curve_bbox(Geom::Point const p000, Geom::Point const p001,
	      Geom::Point const p011, Geom::Point const p111,
	      Geom::Rect r)
{
	using Geom::X;
	using Geom::Y;

	for(unsigned dim = 0; dim < 2; dim++)
		nr_curve_bbox(p000[dim], p001[dim], 
			      p011[dim], p111[dim], 
			      bbox.x0, bbox.x1);
}

/* Fast bbox calculation */
/* Thanks to Nathan Hurst for suggesting it */

static void
nr_curve_bbox (Geom::Coord x000, Geom::Coord x001, Geom::Coord x011, Geom::Coord x111, Geom::Coord lo, Geom::Coord hi)
{
	Geom::Coord a, b, c, D;

	lo = (Geom::Coord) MIN (lo, x111);
	hi = (Geom::Coord) MAX (hi, x111);

	/*
	 * xttt = s * (s * (s * x000 + t * x001) + t * (s * x001 + t * x011)) + t * (s * (s * x001 + t * x011) + t * (s * x011 + t * x111))
	 * xttt = s * (s2 * x000 + s * t * x001 + t * s * x001 + t2 * x011) + t * (s2 * x001 + s * t * x011 + t * s * x011 + t2 * x111)
	 * xttt = s * (s2 * x000 + 2 * st * x001 + t2 * x011) + t * (s2 * x001 + 2 * st * x011 + t2 * x111)
	 * xttt = s3 * x000 + 2 * s2t * x001 + st2 * x011 + s2t * x001 + 2st2 * x011 + t3 * x111
	 * xttt = s3 * x000 + 3s2t * x001 + 3st2 * x011 + t3 * x111
	 * xttt = s3 * x000 + (1 - s) 3s2 * x001 + (1 - s) * (1 - s) * 3s * x011 + (1 - s) * (1 - s) * (1 - s) * x111
	 * xttt = s3 * x000 + (3s2 - 3s3) * x001 + (3s - 6s2 + 3s3) * x011 + (1 - 2s + s2 - s + 2s2 - s3) * x111
	 * xttt = (x000 - 3 * x001 + 3 * x011 -     x111) * s3 +
	 *        (       3 * x001 - 6 * x011 + 3 * x111) * s2 +
	 *        (                  3 * x011 - 3 * x111) * s  +
	 *        (                                 x111)
	 * xttt' = (3 * x000 - 9 * x001 +  9 * x011 - 3 * x111) * s2 +
	 *         (           6 * x001 - 12 * x011 + 6 * x111) * s  +
	 *         (                       3 * x011 - 3 * x111)
	 */

	a = 3 * x000 - 9 * x001 + 9 * x011 - 3 * x111;
	b = 6 * x001 - 12 * x011 + 6 * x111;
	c = 3 * x011 - 3 * x111;

	/*
	 * s = (-b +/- sqrt (b * b - 4 * a * c)) / 2 * a;
	 */
	if (fabs (a) < Geom_EPSILON) {
		/* s = -c / b */
		if (fabs (b) > Geom_EPSILON) {
			double s, t, xttt;
			s = -c / b;
			if ((s > 0.0) && (s < 1.0)) {
				t = 1.0 - s;
				xttt = s * s * s * x000 + 3 * s * s * t * x001 + 3 * s * t * t * x011 + t * t * t * x111;
				lo = (float) MIN (lo, xttt);
				hi = (float) MAX (hi, xttt);
			}
		}
	} else {
		/* s = (-b +/- sqrt (b * b - 4 * a * c)) / 2 * a; */
		D = b * b - 4 * a * c;
		if (D >= 0.0) {
			Geom::Coord d, s, t, xttt;
			/* Have solution */
			d = sqrt (D);
			s = (-b + d) / (2 * a);
			if ((s > 0.0) && (s < 1.0)) {
				t = 1.0 - s;
				xttt = s * s * s * x000 + 3 * s * s * t * x001 + 3 * s * t * t * x011 + t * t * t * x111;
				lo = (Geom::Coord) MIN (lo, xttt);
				hi = (Geom::Coord) MAX (hi, xttt);
			}
			s = (-b - d) / (2 * a);
			if ((s > 0.0) && (s < 1.0)) {
				t = 1.0 - s;
				xttt = s * s * s * x000 + 3 * s * s * t * x001 + 3 * s * t * t * x011 + t * t * t * x111;
				lo = (Geom::Coord) MIN (lo, xttt);
				hi = (Geom::Coord) MAX (hi, xttt);
			}
		}
	}
}

