#define __Geom_RECT_C__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include "rect-l.h"

/**
 *    \param r0 Rectangle.
 *    \param r1 Another rectangle.
 *    \param d Filled in with the intersection of r0 and r1.
 *    \return d.
 */


namespace Geom {

Rect::Rect(const Point &p0, const Point &p1)
: _min(MIN(p0[X], p1[X]), MIN(p0[Y], p1[Y])),
  _max(MAX(p0[X], p1[X]), MAX(p0[Y], p1[Y])) {}

/** returns the four corners of the rectangle in the correct winding order */
Point Rect::corner(unsigned i) const {
	switch (i % 4) {
	case 0:
		return _min;
	case 1:
		return Point(_max[X], _min[Y]);
	case 2:
		return _max;
	default: /* i.e. 3 */
		return Point(_min[X], _max[Y]);
	}
}

/** returns the midpoint of this rectangle */
Point Rect::midpoint() const {
	return ( _min + _max ) / 2;
}

/** returns a vector from topleft to bottom right. */
Point Rect::dimensions() const {
	return _max - _min;
}

/** Translates the rectangle by p. */
void Rect::offset(Point p) {
	_min += p;
	_max += p;
}

/** Makes this rectangle large enough to include the point p. */
void Rect::expandTo(Point p) {
	for ( int i=0 ; i < 2 ; i++ ) {
		_min[i] = MIN(_min[i], p[i]);
		_max[i] = MAX(_max[i], p[i]);
	}
}

/** Returns the set of points shared by both rectangles. */
Maybe<Rect> Rect::intersection(const Rect &a, const Rect &b) {
	Rect r;
	for ( int i=0 ; i < 2 ; i++ ) {
		r._min[i] = MAX(a._min[i], b._min[i]);
		r._max[i] = MIN(a._max[i], b._max[i]);

		if ( r._min[i] > r._max[i] ) {
			return Nothing();
		}
	}
	return r;
}

/** returns the smallest rectangle containing both rectangles */
Rect Rect::union_bounds(const Rect &a, const Rect &b) {
	Rect r;
	for ( int i=0; i < 2 ; i++ ) {
		r._min[i] = MIN(a._min[i], b._min[i]);
		r._max[i] = MAX(a._max[i], b._max[i]);
	}
	return r;
}

}  // namespace Geom


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
