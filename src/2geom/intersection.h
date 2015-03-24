/**
 * \file
 * \brief Intersection utilities
 *//*
 * Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * 
 * Copyright 2015 Authors
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

#ifndef SEEN_LIB2GEOM_INTERSECTION_H
#define SEEN_LIB2GEOM_INTERSECTION_H

#include <2geom/coord.h>
#include <2geom/point.h>

namespace Geom {

template <typename T>
struct ShapeTraits {
    typedef Coord TimeType;
    typedef typename CoordTraits<T>::IntervalType IntervalType;
};

/** @brief Reference to a shape and a position on it.
 *
 * This holds a reference to a parametric 2D shape - basically anything that has
 * a pointAt method - and a time value. These determine a point on the plane:
 * the result of evaluating the function at that time value.
 */
template <typename T>
class ShapePosition {
public:
    typedef typename ShapeTraits<T>::TimeType TimeType;

    ShapePosition(T const &s, TimeType const &pos)
        : _shape(&s)
        , _pos(pos)
    {}

    TimeType time() const {
        return _pos;
    }
    /// Alias for use with Path and PathVector.
    TimeType position() const {
        return _pos;
    }

    T const &shape() const {
        return *_shape;
    }
    /// Evaluate the shape at the stored time value.
    Point point() const {
        return _shape->pointAt(_pos);
    }

private:
    T const *_shape;
    TimeType _pos;
};

/** @brief Intersection between two shapes.
 */
template <typename TA, typename TB = TA>
class Intersection
{
public:
    typedef typename ShapeTraits<TA>::TimeType TimeA;
    typedef typename ShapeTraits<TB>::TimeType TimeB;

    /** @brief Construct from shape references and time values.
     * By default, the intersection point will be halfway between the evaluated
     * points on the two shapes. */
    Intersection(TA const &sa, TB const &sb, Coord ta, Coord tb)
        : first(sa, ta)
        , second(sb, tb)
        , _point(lerp(0.5, first.point(), second.point()))
    {}

    /// Additionally report the intersection point.
    Intersection(TA const &sa, TB const &sb, Coord ta, Coord tb, Point const &p)
        : first(sa, ta)
        , second(sb, tb)
        , _point(p)
    {}

    /// Intersection point, as calculated by the intersection algorithm.
    Point point() const {
        return _point;
    }
    /// Implicit conversion to Point.
    operator Point() const {
        return _point;
    }
    /// Rough estimate of the precision of intersection.
    Coord delta() const {
        return distance(first.point(), second.point());
    }

public:
    /// First shape and time value.
    ShapePosition<TA> first;
    /// Second shape and time value.
    ShapePosition<TB> second;
private:
    // Recalculation of the intersection point from the time values is in many cases
    // less precise than the value obtained directly from the intersection algorithm,
    // so we need to store it.
    Point _point;
};

} // namespace Geom

#endif // SEEN_LIB2GEOM_INTERSECTION_H
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
