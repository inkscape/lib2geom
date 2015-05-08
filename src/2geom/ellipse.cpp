/** @file
 * @brief Ellipse shape
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

#include <2geom/ellipse.h>
#include <2geom/svg-elliptical-arc.h>
#include <2geom/numeric/fitting-tool.h>
#include <2geom/numeric/fitting-model.h>

namespace Geom {

void Ellipse::setCoefficients(double A, double B, double C, double D, double E, double F)
{
    double den = 4*A*C - B*B;
    if (den == 0) {
        THROW_RANGEERROR("den == 0, while computing ellipse centre");
    }
    _center[X] = (B*E - 2*C*D) / den;
    _center[Y] = (B*D - 2*A*E) / den;

    // evaluate the a coefficient of the ellipse equation in normal form
    // E(x,y) = a*(x-cx)^2 + b*(x-cx)*(y-cy) + c*(y-cy)^2 = 1
    // where b = a*B , c = a*C, (cx,cy) == centre
    double num =   A * sqr(_center[X])
                 + B * _center[X] * _center[Y]
                 + C * sqr(_center[Y])
                 - F;


    //evaluate ellipse rotation angle
    double rot = std::atan2( -B, -(A - C) )/2;
//      std::cerr << "rot = " << rot << std::endl;
    bool swap_axes = false;

    if (rot >= M_PI/2 || rot < 0) {
        swap_axes = true;
    }

    // evaluate the length of the ellipse rays
    double sinrot, cosrot;
    sincos(rot, sinrot, cosrot);
    double cos2 = cosrot * cosrot;
    double sin2 = sinrot * sinrot;
    double cossin = cosrot * sinrot;

    den = A * cos2 + B * cossin + C * sin2;
    if (den == 0) {
        THROW_RANGEERROR("den == 0, while computing 'rx' coefficient");
    }
    double rx2 = num / den;
    if (rx2 < 0) {
        THROW_RANGEERROR("rx2 < 0, while computing 'rx' coefficient");
    }
    double rx = std::sqrt(rx2);

    den = C * cos2 - B * cossin + A * sin2;
    if (den == 0) {
        THROW_RANGEERROR("den == 0, while computing 'ry' coefficient");
    }
    double ry2 = num / den;
    if (ry2 < 0) {
        THROW_RANGEERROR("ry2 < 0, while computing 'rx' coefficient");
    }
    double ry = std::sqrt(ry2);

    // the solution is not unique so we choose always the ellipse
    // with a rotation angle between 0 and PI/2
    if (swap_axes) {
        std::swap(rx, ry);
    }

    if (rx == ry) {
        rot = 0;
    }
    if (rot < 0) {
        rot += M_PI/2;
    }

    _rays[X] = rx;
    _rays[Y] = ry;
    _angle = rot;
}


Affine Ellipse::unitCircleTransform() const
{
    Affine ret = Scale(ray(X), ray(Y)) * Rotate(_angle);
    ret.setTranslation(center());
    return ret;
}


std::vector<double> Ellipse::coefficients() const
{
    if (ray(X) == 0 || ray(Y) == 0) {
        THROW_RANGEERROR("a degenerate ellipse doesn't have an implicit form");
    }

    std::vector<double> coeff(6);
    double cosrot, sinrot;
    sincos(_angle, sinrot, cosrot);
    double cos2 = cosrot * cosrot;
    double sin2 = sinrot * sinrot;
    double cossin = cosrot * sinrot;
    double invrx2 = 1 / (ray(X) * ray(X));
    double invry2 = 1 / (ray(Y) * ray(Y));

    coeff[0] = invrx2 * cos2 + invry2 * sin2;
    coeff[1] = 2 * (invrx2 - invry2) * cossin;
    coeff[2] = invrx2 * sin2 + invry2 * cos2;
    coeff[3] = -(2 * coeff[0] * center(X) + coeff[1] * center(Y));
    coeff[4] = -(2 * coeff[2] * center(Y) + coeff[1] * center(X));
    coeff[5] = coeff[0] * center(X) * center(X)
             + coeff[1] * center(X) * center(Y)
             + coeff[2] * center(Y) * center(Y)
             - 1;
    return coeff;
}


void Ellipse::fit(std::vector<Point> const &points)
{
    size_t sz = points.size();
    if (sz < 5) {
        THROW_RANGEERROR("fitting error: too few points passed");
    }
    NL::LFMEllipse model;
    NL::least_squeares_fitter<NL::LFMEllipse> fitter(model, sz);

    for (size_t i = 0; i < sz; ++i) {
        fitter.append(points[i]);
    }
    fitter.update();

    NL::Vector z(sz, 0.0);
    model.instance(*this, fitter.result(z));
}


EllipticalArc *
Ellipse::arc(Point const &ip, Point const &inner, Point const &fp,
             bool _svg_compliant)
{
    // This is resistant to degenerate ellipses:
    // both flags evaluate to false in that case.

    bool large_arc_flag = false;
    bool sweep_flag = false;

    // Determination of large arc flag:
    // The arc is larger than half of the ellipse if the inner point
    // is on the same side of the line going from the initial
    // to the final point as the center of the ellipse
    Point versor = fp - ip;
    double sdist_c = cross(versor, _center - ip);
    double sdist_inner = cross(versor, inner - ip);

    // if we have exactly half of an arc, do not set the large flag.
    if (sdist_c != 0 && sgn(sdist_c) == sgn(sdist_inner)) {
        large_arc_flag = true;
    }

    // Determination of sweep flag:
    // If the inner point is on the left side of the ip-fp line,
    // we go in clockwise direction.
    if (sdist_inner < 0) {
        sweep_flag = true;
    }

    EllipticalArc *ret_arc;
    if (_svg_compliant) {
        ret_arc = new SVGEllipticalArc(ip, ray(X), ray(Y), rotationAngle(),
                                       large_arc_flag, sweep_flag, fp);
    } else {
        ret_arc = new EllipticalArc(ip, ray(X), ray(Y), rotationAngle(),
                                    large_arc_flag, sweep_flag, fp);
    }
    return ret_arc;
}

Ellipse &Ellipse::operator*=(Rotate const &r)
{
    _angle += r.angle();
    // keep the angle in the first quadrant
    if (_angle < 0) {
        _angle += M_PI;
    }
    if (_angle >= M_PI/2) {
        std::swap(_rays[X], _rays[Y]);
        _angle -= M_PI/2;
    }
    _center *= r;
    return *this;
}

Ellipse &Ellipse::operator*=(Affine const& m)
{
    Affine a = Scale(ray(X), ray(Y)) * Rotate(_angle);
    Affine mwot = m.withoutTranslation();
    Affine am = a * mwot;
    Point new_center = _center * m;

    if (are_near(am.descrim(), 0)) {
        double angle;
        if (am[0] != 0) {
            angle = std::atan2(am[2], am[0]);
        } else if (am[1] != 0) {
            angle = std::atan2(am[3], am[1]);
        } else {
            angle = M_PI/2;
        }
        Point v = Point::polar(angle) * am;
        _center = new_center;
        _rays[X] = L2(v);
        _rays[Y] = 0;
        _angle = atan2(v);
        return *this;
    }

    std::vector<double> coeff = coefficients();
    Affine q( coeff[0],   coeff[1]/2,
              coeff[1]/2, coeff[2],
              0,          0   );

    Affine invm = mwot.inverse();
    q = invm * q ;
    std::swap(invm[1], invm[2]);
    q *= invm;
    setCoefficients(q[0], 2*q[1], q[3], 0, 0, -1);
    _center = new_center;

    return *this;
}

Ellipse::Ellipse(Geom::Circle const &c)
{
    _center = c.center();
    _rays[X] = _rays[Y] = c.radius();
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


