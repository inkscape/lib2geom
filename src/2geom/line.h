/**
 * \file
 * \brief  Infinite straight line
 *//*
 * Authors:
 *   Marco Cecchetti <mrcekets at gmail.com>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * Copyright 2008-2011 Authors
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 */

#ifndef LIB2GEOM_SEEN_LINE_H
#define LIB2GEOM_SEEN_LINE_H

#include <cmath>
#include <boost/optional.hpp>
#include <2geom/bezier-curve.h> // for LineSegment
#include <2geom/rect.h>
#include <2geom/crossing.h>
#include <2geom/exception.h>
#include <2geom/ray.h>
#include <2geom/angle.h>
#include <2geom/intersection.h>

namespace Geom
{

class Line
    : boost::equality_comparable1<Line
    , MultipliableNoncommutative<Line, Affine
      > >
{
private:
    Point _initial;
    Point _final;
public:
    /// @name Creating lines.
    /// @{
    /** @brief Create a default horizontal line.
     * Creates a line with unit speed going in +X direction. */
    Line()
        : _initial(0,0), _final(1,0)
    {}
    /** @brief Create a line with the specified inclination.
     * @param origin One of the points on the line
     * @param angle Angle of the line in mathematical convention */
    Line(Point const &origin, Coord angle)
        : _initial(origin)
    {
        Point v;
        sincos(angle, v[Y], v[X]);
        _final = _initial + v;
    }

    /** @brief Create a line going through two points.
     * The first point will be at time 0, while the second one
     * will be at time 1.
     * @param a Initial point
     * @param b First point */
    Line(Point const &a, Point const &b)
        : _initial(a)
        , _final(b)
    {}

    /** @brief Create a line based on the coefficients of its equation.
     @see Line::setCoefficients() */
    Line(double a, double b, double c) {
        setCoefficients(a, b, c);
    }

    /** @brief Create a line by extending a line segment. */
    explicit Line(LineSegment const &seg)
        : _initial(seg.initialPoint())
        , _final(seg.finalPoint())
    {}

    /** @brief Create a line by extending a ray. */
    explicit Line(Ray const &r)
        : _initial(r.origin())
        , _final(r.origin() + r.versor())
    {}

    /** @brief Create a line normal to a vector at a specified distance from origin. */
    static Line from_normal_distance(Point const &n, Coord c) {
        Point start = c * n.normalized();
        Line l(start, start + rot90(n));
        return l;
    }
    /** @brief Create a line from origin and unit vector.
     * Note that each line direction has two possible unit vectors.
     * @param o Point through which the line will pass
     * @param v Unit vector of the line's direction */
    static Line from_origin_and_versor(Point const &o, Point const &v) {
        Line l(o, o + v);
        return l;
    }

    Line* duplicate() const {
        return new Line(*this);
    }
    /// @}

    /// @name Retrieve and set the line's parameters.
    /// @{
    /** @brief Get the line's origin point. */
    Point origin() const { return _initial; }
    /** @brief Get the line's direction unit vector. */
    Point versor() const { return _final - _initial; }
    // return the angle described by rotating the X-axis in cw direction
    // until it overlaps the line
    // the returned value is in the interval [0, PI[
    Coord angle() const {
        Point d = _final - _initial;
        double a = std::atan2(d[Y], d[X]);
        if (a < 0) a += M_PI;
        if (a == M_PI) a = 0;
        return a;
    }

    /** @brief Set the point at zero time.
     * The orientation remains unchanged, modulo numeric errors during addition. */
    void setOrigin(Point const &p) {
        Point d = p - _initial;
        _initial = p;
        _final += d;
    }
    /** @brief Set the speed of the line.
     * Origin remains unchanged. */
    void setVersor(Point const &v) {
        _final = _initial + v;
    }

    void setAngle(Coord angle) {
        Point v;
        sincos(angle, v[Y], v[X]);
        v *= distance(_initial, _final);
        _final = _initial + v;
    }

    /** @brief Set a line based on two points it should pass through. */
    void setPoints(Point const &a, Point const &b) {
        _initial = a;
        _final = b;
    }

    void setCoefficients(double a, double b, double c);
    std::vector<double> coefficients() const;
    void coefficients(Coord &a, Coord &b, Coord &c) const;

    /** @brief Check if the line has more than one point.
     * A degenerate line can be created if the line is created from a line equation
     * that has no solutions.
     * @return True if the line has no points or exactly one point */
    bool isDegenerate() const {
        return _initial == _final;
    }

    /** @brief Reparametrize the line so that it has unit speed. */
    void normalize() {
        Point v = _final - _initial;
        v.normalize();
        _final = _initial + v;
    }
    /** @brief Return a new line reparametrized for unit speed. */
    Line normalized() const {
        Point v = _final - _initial;
        v.normalize();
        Line ret(_initial, _initial + v);
        return ret;
    }
    /// @}

    /// @name Evaluate the line as a function.
    ///@{
    Point initialPoint() const {
        return _initial;
    }
    Point finalPoint() const {
        return _final;
    }
    Point pointAt(Coord t) const {
        return lerp(t, _initial, _final);;
    }

    Coord valueAt(Coord t, Dim2 d) const {
        return lerp(t, _initial[d], _final[d]);
    }

    Coord timeAt(Point const &p) const;

    /** @brief Get a time value corresponding to a projection of a point on the line.
     * @param p Arbitrary point.
     * @return Time value corresponding to a point closest to @c p. */
    Coord timeAtProjection(Point const& p) const {
        if ( isDegenerate() ) return 0;
        Point v = versor();
        return dot(p - _initial, v) / dot(v, v);
    }

    /** @brief Find a point on the line closest to the query point.
     * This is an alias for timeAtProjection(). */
    Coord nearestTime(Point const &p) const {
        return timeAtProjection(p);
    }

    std::vector<Coord> roots(Coord v, Dim2 d) const;
    Coord root(Coord v, Dim2 d) const;
    /// @}

    /// @name Create other objects based on this line.
    /// @{
    void reverse() {
        std::swap(_final, _initial);
    }
    /** @brief Create a line containing the same points, but with negated time values.
     * @return Line \f$g\f$ such that \f$g(t) = f(-t)\f$ */
    Line reversed() const {
        Line result(_final, _initial);
        return result;
    }

    /** @brief Same as segment(), but allocate the line segment dynamically. */
    // TODO remove this?
    Curve* portion(Coord  f, Coord t) const {
        LineSegment* seg = new LineSegment(pointAt(f), pointAt(t));
        return seg;
    }

    /** @brief Create a segment of this line.
     * @param f Time value for the initial point of the segment
     * @param t Time value for the final point of the segment
     * @return Created line segment */
    LineSegment segment(Coord  f, Coord t) const {
        return LineSegment(pointAt(f), pointAt(t));
    }

    /// Return the portion of the line that is inside the given rectangle
    boost::optional<LineSegment> clip(Rect const &r) const;

    /** @brief Create a ray starting at the specified time value.
     * The created ray will go in the direction of the line's versor (in the direction
     * of increasing time values).
     * @param t Time value where the ray should start
     * @return Ray starting at t and going in the direction of the versor */
    Ray ray(Coord t) {
        Ray result;
        result.setOrigin(pointAt(t));
        result.setVersor(versor());
        return result;
    }

    /** @brief Create a derivative of the line.
     * The new line will always be degenerate. Its origin will be equal to this
     * line's versor. */
    Line derivative() const {
        Point v = versor();
        Line result(v, v);
        return result;
    }

    /** @brief Create a line transformed by an affine transformation. */
    Line transformed(Affine const& m) const {
        return Line(_initial * m, _final * m);
    }

    /** @brief Get a unit vector normal to the line.
     * If Y grows upwards, then this is the left normal. If Y grows downwards,
     * then this is the right normal. */
    Point normal() const {
        return rot90(versor()).normalized();
    }

    // what does this do?
    Point normalAndDist(double & dist) const {
        Point n = normal();
        dist = -dot(n, _initial);
        return n;
    }
    /// @}

    //std::vector<LineIntersection> intersect(Line const &other, Coord precision = EPSILON) const;

    Line &operator*=(Affine const &m) {
        _initial *= m;
        _final *= m;
        return *this;
    }
    bool operator==(Line const &other) const {
        if (distance(pointAt(nearestTime(other._initial)), other._initial) != 0) return false;
        if (distance(pointAt(nearestTime(other._final)), other._final) != 0) return false;
        return true;
    }
}; // end class Line


inline
double distance(Point const &p, Line const &line)
{
    if (line.isDegenerate()) {
        return ::Geom::distance(p, line.initialPoint());
    } else {
        Coord t = line.nearestTime(p);
        return ::Geom::distance(line.pointAt(t), p);
    }
}

inline
bool are_near(Point const &p, Line const &line, double eps = EPSILON)
{
    return are_near(distance(p, line), 0, eps);
}

inline
bool are_parallel(Line const &l1, Line const &l2, double eps = EPSILON)
{
    return are_near(cross(l1.versor(), l2.versor()), 0, eps);
}

inline
bool are_same(Line const &l1, Line const &l2, double eps = EPSILON)
{
    return are_parallel(l1, l2, eps) && are_near(l1.origin(), l2, eps);
}

inline
bool are_orthogonal(Line const &l1, Line const &l2, double eps = EPSILON)
{
    return are_near(dot(l1.versor(), l2.versor()), 0, eps);
}

inline
bool are_collinear(Point const& p1, Point const& p2, Point const& p3,
                   double eps = EPSILON)
{
    return are_near( cross(p3, p2) - cross(p3, p1) + cross(p2, p1), 0, eps);
}

// evaluate the angle between l1 and l2 rotating l1 in cw direction
// until it overlaps l2
// the returned value is an angle in the interval [0, PI[
inline
double angle_between(Line const& l1, Line const& l2)
{
    double angle = angle_between(l1.versor(), l2.versor());
    if (angle < 0) angle += M_PI;
    if (angle == M_PI) angle = 0;
    return angle;
}

inline
double distance(Point const &p, LineSegment const &seg)
{
    double t = seg.nearestTime(p);
    return distance(p, seg.pointAt(t));
}

inline
bool are_near(Point const &p, LineSegment const &seg, double eps = EPSILON)
{
    return are_near(distance(p, seg), 0, eps);
}

// build a line passing by _point and orthogonal to _line
inline
Line make_orthogonal_line(Point const &p, Line const &line)
{
    Point d = line.versor().cw();
    Line l(p, p + d);
    return l;
}

// build a line passing by _point and parallel to _line
inline
Line make_parallel_line(Point const &p, Line const &line)
{
    Line result(line);
    result.setOrigin(p);
    return result;
}

// build a line passing by the middle point of _segment and orthogonal to it.
inline
Line make_bisector_line(LineSegment const& _segment)
{
    return make_orthogonal_line( middle_point(_segment), Line(_segment) );
}

// build the bisector line of the angle between ray(O,A) and ray(O,B)
inline
Line make_angle_bisector_line(Point const &A, Point const &O, Point const &B)
{
    AngleInterval ival(Angle(A-O), Angle(B-O));
    Angle bisect = ival.angleAt(0.5);
    return Line(O, bisect);
}

// prj(P) = rot(v, Point( rot(-v, P-O)[X], 0 )) + O
inline
Point projection(Point const &p, Line const &line)
{
    return line.pointAt(line.nearestTime(p));
}

inline
LineSegment projection(LineSegment const &seg, Line const &line)
{
    return line.segment(line.nearestTime(seg.initialPoint()),
                        line.nearestTime(seg.finalPoint()));
}

inline
boost::optional<LineSegment> clip(Line const &l, Rect const &r) {
    return l.clip(r);
}


namespace detail
{

OptCrossing intersection_impl(Ray const& r1, Line const& l2, unsigned int i);
OptCrossing intersection_impl( LineSegment const& ls1,
                             Line const& l2,
                             unsigned int i );
OptCrossing intersection_impl( LineSegment const& ls1,
                             Ray const& r2,
                             unsigned int i );
}


inline
OptCrossing intersection(Ray const& r1, Line const& l2)
{
    return detail::intersection_impl(r1,  l2, 0);

}

inline
OptCrossing intersection(Line const& l1, Ray const& r2)
{
    return detail::intersection_impl(r2,  l1, 1);
}

inline
OptCrossing intersection(LineSegment const& ls1, Line const& l2)
{
    return detail::intersection_impl(ls1,  l2, 0);
}

inline
OptCrossing intersection(Line const& l1, LineSegment const& ls2)
{
    return detail::intersection_impl(ls2,  l1, 1);
}

inline
OptCrossing intersection(LineSegment const& ls1, Ray const& r2)
{
    return detail::intersection_impl(ls1,  r2, 0);

}

inline
OptCrossing intersection(Ray const& r1, LineSegment const& ls2)
{
    return detail::intersection_impl(ls2,  r1, 1);
}


OptCrossing intersection(Line const& l1, Line const& l2);

OptCrossing intersection(Ray const& r1, Ray const& r2);

OptCrossing intersection(LineSegment const& ls1, LineSegment const& ls2);


} // end namespace Geom


#endif // LIB2GEOM_SEEN_LINE_H


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
