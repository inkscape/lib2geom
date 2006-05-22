#ifndef LIBGeom_Geom_RECT_H_SEEN
#define LIBGeom_Geom_RECT_H_SEEN

/** \file
 * Definitions of NRRect and Geom::Rect types, and some associated functions \& macros.
 */
/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Nathan Hurst <njh@mail.csse.monash.edu.au>
 *   MenTaLguY <mental@rydia.net>
 *
 * This code is in public domain
 */


#include <stdexcept>

#include "point.h"
#include "point-ops.h"
#include "maybe.h"
#include "macros.h"
#include "matrix.h"
#include "point-matrix-ops.h"

namespace Geom {

/** A rectangle is always aligned to the X and Y axis.  This means it
 * can be defined using only 4 coordinates, and determining
 * intersection is very efficient.  The points inside a rectangle are
 * min[dim] <= _pt[dim] <= max[dim].  Emptiness, however, is defined
 * as having zero area, meaning an empty rectangle may still contain
 * points.  Infinities are also permitted. */
class Rect {
public:
    Rect(Rect const &r) : _min(r._min), _max(r._max) {}
    Rect(Point const &p0, Point const &p1);

    Point const &min() const { return _min; }
    Point const &max() const { return _max; }

    /** returns the four corners of the rectangle in order
     *  (clockwise if +Y is up, anticlockwise if +Y is down) */
    Point corner(unsigned i) const;

    /** returns a vector from min to max. */
    Point dimensions() const;

    /** returns the midpoint of this rect. */
    Point midpoint() const;

    /** does this rectangle have zero area? */
    bool isEmpty() const {
        return isEmpty<X>() || isEmpty<Y>();
    }

    bool intersects(Rect const &r) const {
        return intersects<X>(r) && intersects<Y>(r);
    }
    bool contains(Rect const &r) const {
        return contains<X>(r) && contains<Y>(r);
    }
    bool contains(Point const &p) const {
        return contains<X>(p) && contains<Y>(p);
    }

    double area() const {
        return extent<X>() * extent<Y>();
    }

    double maxExtent() const {
        return MAX(extent<X>(), extent<Y>());
    }

    double extent(Dim2 const axis) const {
        switch (axis) {
            case X: return extent<X>();
            case Y: return extent<Y>();
            default: throw;//("invalid axis value %d", (int) axis); return 0;
        };
    }

    double extent(unsigned i) const throw(std::out_of_range) {
        switch (i) {
            case 0: return extent<X>();
            case 1: return extent<Y>();
            default: throw std::out_of_range("Dimension out of range");
        };
    }

    /**
        \brief  Remove some precision from the Rect
        \param  places  The number of decimal places left in the end

        This function just calls round on the \c _min and \c _max points.
    */
    inline void round(int places = 0) {
        _min.round(places);
        _max.round(places);
        return;
    }

    /** Translates the rectangle by p. */
    void offset(Point p);

    /** Makes this rectangle large enough to include the point p. */
    void expandTo(Point p);

    /** Makes this rectangle large enough to include the rectangle r. */
    void expandTo(Rect const &r);

    inline void move_left (double by) {
        _min[Geom::X] += by;
    }
    inline void move_right (double by) {
        _max[Geom::X] += by;
    }
    inline void move_top (double by) {
        _min[Geom::Y] += by;
    }
    inline void move_bottom (double by) {
        _max[Geom::Y] += by;
    }

    /** Returns the set of points shared by both rectangles. */
    static Maybe<Rect> intersection(Rect const &a, Rect const &b);

    /** Returns the smallest rectangle that encloses both rectangles. */
    static Rect union_bounds(Rect const &a, Rect const &b);

    /** Scales the rect by s, with origin at 0, 0 */
    inline Rect operator*(double const s) const {
        return Rect(s * min(), s * max());
    }

    /** Transforms the rect by m. Note that it gives correct results only for scales and translates */
    inline Rect operator*(Matrix const m) const {
        return Rect(_min * m, _max * m);
    }

    inline bool operator==(Rect const &in_rect) {
        return ((this->min() == in_rect.min()) && (this->max() == in_rect.max()));
    }

    friend inline std::ostream &operator<<(std::ostream &out_file, Geom::Rect const &in_rect);

private:
    Rect() {}

    template <Geom::Dim2 axis>
    double extent() const {
        return _max[axis] - _min[axis];
    }

    template <Dim2 axis>
    bool isEmpty() const {
        return !( _min[axis] < _max[axis] );
    }

    template <Dim2 axis>
    bool intersects(Rect const &r) const {
        return _max[axis] >= r._min[axis] && _min[axis] <= r._max[axis];
    }

    template <Dim2 axis>
    bool contains(Rect const &r) const {
        return contains(r._min) && contains(r._max);
    }

    template <Dim2 axis>
    bool contains(Point const &p) const {
        return p[axis] >= _min[axis] && p[axis] <= _max[axis];
    }

    Point _min, _max;

    /* evil, but temporary */
    friend class Maybe<Rect>;
};

#if 0
/** A function to print out the rectange if sent to an output
    stream. */
inline std::ostream
&operator<<(std::ostream &out_file, Geom::Rect const &in_rect)
{
    out_file << "Rectangle:\n";
    out_file << "\tMin Point -> " << in_rect.min() << "\n";
    out_file << "\tMax Point -> " << in_rect.max() << "\n";

    return out_file;
}
#endif

} /* namespace Geom */


#endif /* !LIBGeom_Geom_RECT_H_SEEN */

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
