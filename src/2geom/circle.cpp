/** @file
 * @brief Circle shape
 *//*
 * Authors:
 *   Marco Cecchetti <mrcekets at gmail.com>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright 2008-2014 Authors
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

#include <2geom/circle.h>
#include <2geom/ellipse.h>
#include <2geom/elliptical-arc.h>
#include <2geom/numeric/fitting-tool.h>
#include <2geom/numeric/fitting-model.h>

namespace Geom {

void Circle::setCoefficients(Coord A, Coord B, Coord C, Coord D)
{
    if (A == 0) {
        THROW_RANGEERROR("square term coefficient == 0");
    }

    //std::cerr << "B = " << B << "  C = " << C << "  D = " << D << std::endl;

    Coord b = B / A;
    Coord c = C / A;
    Coord d = D / A;

    _center[X] = -b/2;
    _center[Y] = -c/2;
    Coord r2 = _center[X] * _center[X] + _center[Y] * _center[Y] - d;

    if (r2 < 0) {
        THROW_RANGEERROR("ray^2 < 0");
    }

    _radius = std::sqrt(r2);
}

void Circle::coefficients(Coord &A, Coord &B, Coord &C, Coord &D) const
{
    A = 1;
    B = -2 * _center[X];
    C = -2 * _center[Y];
    D = _center[X] * _center[X] + _center[Y] * _center[Y] - _radius * _radius;
}

std::vector<Coord> Circle::coefficients() const
{
    std::vector<Coord> c(4);
    coefficients(c[0], c[1], c[2], c[3]);
    return c;
}


Zoom Circle::unitCircleTransform() const
{
    Zoom ret(_radius, _center / _radius);
    return ret;
}

Zoom Circle::inverseUnitCircleTransform() const
{
    if (_radius == 0) {
        THROW_RANGEERROR("degenerate circle does not have an inverse unit circle transform");
    }

    Zoom ret(1/_radius, Translate(-_center));
    return ret;
}


Point Circle::pointAt(Coord t) const {
    return _center + Point::polar(t) * _radius;
}

Coord Circle::valueAt(Coord t, Dim2 d) const {
    Coord delta = (d == X ? std::cos(t) : std::sin(t));
    return _center[d] + delta * _radius;
}

Coord Circle::timeAt(Point const &p) const {
    return atan2(p - _center);
}

Coord Circle::nearestTime(Point const &p) const {
    if (_center == p) return 0;
    return timeAt(p);
}

/**
 @param inner a point whose angle with the circle center is inside the angle that the arc spans
 */
EllipticalArc *
Circle::arc(Point const& initial, Point const& inner, Point const& final) const
{
    // TODO native implementation!
    Ellipse e(_center[X], _center[Y], _radius, _radius, 0);
    return e.arc(initial, inner, final);
}

bool Circle::operator==(Circle const &other) const
{
    if (_center != other._center) return false;
    if (_radius != other._radius) return false;
    return true;
}

D2<SBasis> Circle::toSBasis() const
{
    D2<SBasis> B;
    Linear bo = Linear(0, 2 * M_PI);

    B[0] = cos(bo,4);
    B[1] = sin(bo,4);

    B = B * _radius + _center;

    return B;
}


void Circle::fit(std::vector<Point> const& points)
{
    size_t sz = points.size();
    if (sz < 2) {
        THROW_RANGEERROR("fitting error: too few points passed");
    }
    if (sz == 2) {
        _center = points[0] * 0.5 + points[1] * 0.5;
        _radius = distance(points[0], points[1]) / 2;
        return;
    }

    NL::LFMCircle model;
    NL::least_squeares_fitter<NL::LFMCircle> fitter(model, sz);

    for (size_t i = 0; i < sz; ++i) {
        fitter.append(points[i]);
    }
    fitter.update();

    NL::Vector z(sz, 0.0);
    model.instance(*this, fitter.result(z));
}


std::ostream &operator<<(std::ostream &out, Circle const &c)
{
    out << "Circle(" << c.center() << ", " << format_coord_nice(c.radius()) << ")";
    return out;
}

}  // end namespace Geom

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
