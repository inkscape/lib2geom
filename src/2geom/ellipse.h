/** @file
 * @brief Ellipse shape
 *//*
 * Authors:
 *   Marco Cecchetti <mrcekets at gmail.com>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright 2008  authors
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


#ifndef LIB2GEOM_SEEN_ELLIPSE_H
#define LIB2GEOM_SEEN_ELLIPSE_H

#include <vector>
#include <2geom/angle.h>
#include <2geom/exception.h>
#include <2geom/point.h>
#include <2geom/transforms.h>

namespace Geom {

class EllipticalArc;
class Circle;

/** @brief Set of points with a constant sum of distances from two foci
 * @ingroup Shapes */
class Ellipse
    : boost::multipliable< Ellipse, Translate
    , boost::multipliable< Ellipse, Scale
    , boost::multipliable< Ellipse, Rotate
    , boost::multipliable< Ellipse, Zoom
    , boost::multipliable< Ellipse, Affine
      > > > > >
{
    Point _center;
    Point _rays;
    Angle _angle;
public:
    Ellipse() {}
    Ellipse(Point const &c, Point const &r, Coord angle)
        : _center(c)
        , _rays(r)
        , _angle(angle)
    {}
    Ellipse(Coord cx, Coord cy, Coord rx, Coord ry, Coord angle)
        : _center(cx, cy)
        , _rays(rx, ry)
        , _angle(angle)
    {}
    Ellipse(double A, double B, double C, double D, double E, double F) {
        setCoefficients(A, B, C, D, E, F);
    }
    Ellipse(Geom::Circle const &c);

    void set(Point const &c, Point const &r, Coord angle) {
        _center = c;
        _rays = r;
        _angle = angle;
    }
    void set(Coord cx, Coord cy, Coord rx, Coord ry, Coord a) {
        _center[X] = cx;
        _center[Y] = cy;
        _rays[X] = rx;
        _rays[Y] = ry;
        _angle = a;
    }

    // build an ellipse by its implicit equation:
    // Ax^2 + Bxy + Cy^2 + Dx + Ey + F = 0
    void setCoefficients(double A, double B, double C, double D, double E, double F);

    // biuld up the best fitting ellipse wrt the passed points
    // prerequisite: at least 5 points must be passed
    void fit(std::vector<Point> const& points);

    EllipticalArc *arc(Point const &ip, Point const &inner, Point const &fp,
                       bool svg_compliant = true);

    Point center() const { return _center; }
    Coord center(Dim2 d) const { return _center[d]; }
    Point rays() const { return _rays; }
    Coord ray(Dim2 d) const { return _rays[d]; }
    Angle rotationAngle() const { return _angle; }

    /** @brief Compute the transform that maps the unit circle to this ellipse.
     * Each ellipse can be interpreted as a translated, scaled and rotate unit circle.
     * This function returns the transform that maps the unit circle to this ellipse.
     * @return Transform from unit circle to the ellipse */
    Affine unitCircleTransform() const;

    std::vector<double> coefficients() const;

    Ellipse &operator*=(Translate const &t) {
        _center *= t;
        return *this;
    }
    Ellipse &operator*=(Scale const &s) {
        _center *= s;
        _rays *= s;
        return *this;
    }
    Ellipse &operator*=(Zoom const &z) {
        _center *= z;
        _rays *= z.scale();
        return *this;
    }
    Ellipse &operator*=(Rotate const &r);
    Ellipse &operator*=(Affine const &m);
};

} // end namespace Geom

#endif // LIB2GEOM_SEEN_ELLIPSE_H

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
