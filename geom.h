/**
 *  \file geom.h
 *  \brief Various geometrical calculations
 *
 *  Authors:
 *   Nathan Hurst <njh@mail.csse.monash.edu.au>
 *
 * Copyright (C) 1999-2002 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "forward.h"

enum IntersectorKind {
    intersects = 0,
    parallel,
    coincident,
    no_intersection
};

/* Define here various primatives, such as line, line segment, circle, bezier path etc. */


int
intersector_ccw(const Geom::Point& p0, const Geom::Point& p1,
		const Geom::Point& p2);

/* intersectors */

IntersectorKind
line_intersection(Geom::Point const &n0, double const d0,
		  Geom::Point const &n1, double const d1,
		  Geom::Point &result);

IntersectorKind
segment_intersect(Geom::Point const &p00, Geom::Point const &p01,
		  Geom::Point const &p10, Geom::Point const &p11,
		  Geom::Point &result);

IntersectorKind
line_twopoint_intersect(Geom::Point const &p00, Geom::Point const &p01,
			Geom::Point const &p10, Geom::Point const &p11,
			Geom::Point &result);
