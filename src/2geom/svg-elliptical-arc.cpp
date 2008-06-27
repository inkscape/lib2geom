/*
 * SVG Elliptical Arc Class
 *
 * Copyright 2008  Marco Cecchetti <mrcekets at gmail.com>
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


#include "svg-elliptical-arc.h"
#include "bezier-curve.h"
#include "poly.h"

#include <cfloat>
#include <limits>




namespace Geom
{


Rect SVGEllipticalArc::boundsExact() const
{
    Rect result;
    return result;
}


double SVGEllipticalArc::valueAtAngle(Coord t, Dim2 d) const
{
    double sin_rot_angle = std::sin(rotation_angle());
    double cos_rot_angle = std::cos(rotation_angle());
    if ( d == X )
    {
        return    ray(X) * cos_rot_angle * std::cos(t)
                - ray(Y) * sin_rot_angle * std::sin(t)
                + center(X);
    }
    else if ( d == Y )
    {
        return    ray(X) * sin_rot_angle * std::cos(t)
                + ray(Y) * cos_rot_angle * std::sin(t)
                + center(Y);
    }
    THROW_RANGEERROR("dimension parameter out of range");
}


std::vector<double>
SVGEllipticalArc::roots(double v, Dim2 d) const
{
    if ( d > Y )
    {
        THROW_RANGEERROR("dimention out of range");
    }

    std::vector<double> sol;

    return sol;
}


// D(E(t,C),t) = E(t+PI/2,O)
Curve* SVGEllipticalArc::derivative() const
{
    SVGEllipticalArc* result = new SVGEllipticalArc(*this);
    result->m_center[X] = result->m_center[Y] = 0;
    result->m_start_angle += M_PI/2;
    if( !( result->m_start_angle < 2*M_PI ) )
    {
        result->m_start_angle -= 2*M_PI;
    }
    result->m_end_angle += M_PI/2;
    if( !( result->m_end_angle < 2*M_PI ) )
    {
        result->m_end_angle -= 2*M_PI;
    }
    result->m_initial_point = result->pointAtAngle( result->start_angle() );
    result->m_final_point = result->pointAtAngle( result->end_angle() );
    return result;

}


std::vector<Point>
SVGEllipticalArc::pointAndDerivatives(Coord t, unsigned int n) const
{
    unsigned int nn = n+1; // nn represents the size of the result vector.
    std::vector<Point> result;
    result.reserve(nn);
    double angle = map_unit_interval_on_circular_arc(t, start_angle(),
                                                     end_angle(), sweep_flag());
    SVGEllipticalArc ea(*this);
    ea.m_center = Point(0,0);
    unsigned int m = std::min(nn, 4u);
    for ( unsigned int i = 0; i < m; ++i )
    {
        result.push_back( ea.pointAtAngle(angle) );
        angle += M_PI/2;
        if ( !(angle < 2*M_PI) ) angle -= 2*M_PI;
    }
    m = nn / 4;
    for ( unsigned int i = 1; i < m; ++i )
    {
        for ( unsigned int j = 0; j < 4; ++j )
            result.push_back( result[j] );
    }
    m = nn - 4 * m;
    for ( unsigned int i = 0; i < m; ++i )
    {
        result.push_back( result[i] );
    }
    if ( !result.empty() ) // nn != 0
        result[0] = pointAtAngle(angle);
    return result;
}

bool SVGEllipticalArc::containsAngle(Coord angle) const
{
    if ( sweep_flag() )
        if ( start_angle() < end_angle() )
            return ( !( angle < start_angle() || angle > end_angle() ) );
        else
            return ( !( angle < start_angle() && angle > end_angle() ) );
    else
        if ( start_angle() > end_angle() )
            return ( !( angle > start_angle() || angle < end_angle() ) );
        else
            return ( !( angle > start_angle() && angle < end_angle() ) );
}

Curve* SVGEllipticalArc::portion(double f, double t) const
{
    if (f < 0) f = 0;
    if (f > 1) f = 1;
    if (t < 0) t = 0;
    if (t > 1) t = 1;
    if ( are_near(f, t) )
    {
        SVGEllipticalArc* arc = new SVGEllipticalArc();
        arc->m_center = arc->m_initial_point = arc->m_final_point = pointAt(f);
        arc->m_start_angle = arc->m_end_angle = m_start_angle;
        arc->m_rot_angle = m_rot_angle;
        arc->m_sweep = m_sweep;
        arc->m_large_arc = m_large_arc;
    }
    SVGEllipticalArc* arc = new SVGEllipticalArc( *this );
    arc->m_initial_point = pointAt(f);
    arc->m_final_point = pointAt(t);
    double sa = sweep_flag() ? sweep_angle() : -sweep_angle();
    arc->m_start_angle = m_start_angle + sa * f;
    if ( !(arc->m_start_angle < 2*M_PI) )
        arc->m_start_angle -= 2*M_PI;
    if ( arc->m_start_angle < 0 )
        arc->m_start_angle += 2*M_PI;
    arc->m_end_angle = m_start_angle + sa * t;
    if ( !(arc->m_end_angle < 2*M_PI) )
        arc->m_end_angle -= 2*M_PI;
    if ( arc->m_end_angle < 0 )
        arc->m_end_angle += 2*M_PI;
    if ( f > t ) arc->m_sweep = !sweep_flag();
    if ( large_arc_flag() && (arc->sweep_angle() < M_PI) )
        arc->m_large_arc = false;
    return arc;
}


std::vector<double> SVGEllipticalArc::
allNearestPoints( Point const& p, double from, double to ) const
{
    if ( from > to ) std::swap(from, to);
    if ( from < 0 || to > 1 )
    {
        THROW_RANGEERROR("[from,to] interval out of range");
    }
    std::vector<double> result;
    return result;
}


/*
 * NOTE: this implementation follows Standard SVG 1.1 implementation guidelines
 * for elliptical arc curves. See Appendix F.6.
 */
void SVGEllipticalArc::calculate_center_and_extreme_angles()
{
    if ( initialPoint() == finalPoint() )
    {
        m_rx = m_ry = m_rot_angle = m_start_angle = m_end_angle = 0;
        m_center = initialPoint();
        m_large_arc = m_sweep = false;
        return;
    }

    m_rx = std::fabs(m_rx);
    m_ry = std::fabs(m_ry);

    Point d = initialPoint() - finalPoint();

    if ( are_near(ray(X), 0) || are_near(ray(Y), 0) )
    {
        m_rx = L2(d) / 2;
        m_ry = 0;
        m_rot_angle = std::atan2(d[Y], d[X]);
        if (m_rot_angle < 0) m_rot_angle += 2*M_PI;
        m_start_angle = 0;
        m_end_angle = M_PI;
        m_center = middle_point(initialPoint(), finalPoint());
        m_large_arc = false;
        m_sweep = false;
        return;
    }

    double sin_rot_angle = std::sin(rotation_angle());
    double cos_rot_angle = std::cos(rotation_angle());


    Matrix m( cos_rot_angle, -sin_rot_angle,
              sin_rot_angle, cos_rot_angle,
              0,             0              );

    Point p = (d / 2) * m;
    double rx2 = m_rx * m_rx;
    double ry2 = m_ry * m_ry;
    double rxpy = m_rx * p[Y];
    double rypx = m_ry * p[X];
    double rx2py2 = rxpy * rxpy;
    double ry2px2 = rypx * rypx;
    double num = rx2 * ry2;
    double den = rx2py2 + ry2px2;
    assert(den != 0);
    double rad = (num / den) - 1;
    assert(rad >= 0);
    rad = std::sqrt(rad);
    if (m_large_arc == m_sweep) rad = -rad;
    Point c = rad * Point(rxpy / m_ry, -rypx / m_rx);

    m[1] = -m[1];
    m[2] = -m[2];
    m_center = c * m + middle_point(initialPoint(), finalPoint());

    Point sp((p[X] - c[X]) / m_rx, (p[Y] - c[Y]) / m_ry);
    Point ep((-p[X] - c[X]) / m_rx, (-p[Y] - c[Y]) / m_ry);
    Point v(1, 0);
    m_start_angle = angle_between(v, sp);
    double sweep_angle = angle_between(sp, ep);
    if (!m_sweep && sweep_angle > 0) sweep_angle -= 2*M_PI;
    if (m_sweep && sweep_angle < 0) sweep_angle += 2*M_PI;

    if (m_start_angle < 0) m_start_angle += 2*M_PI;
    m_end_angle = m_start_angle + sweep_angle;
    if (m_end_angle < 0) m_end_angle += 2*M_PI;
    if (m_end_angle >= 2*M_PI) m_end_angle -= 2*M_PI;
}


D2<SBasis> SVGEllipticalArc::toSBasis() const
{
    D2<SBasis> arc;
    if ( initialPoint() == finalPoint() )
    {
        return D2<SBasis>(center());
    }
    // the interval of parametrization has to be [0,1]
    Coord et = start_angle() + ( sweep_flag() ? sweep_angle() : -sweep_angle() );
    Linear param(start_angle(), et);
    Coord cos_rot_angle = std::cos(rotation_angle());
    Coord sin_rot_angle = std::sin(rotation_angle());
    // order = 4 seems to be enough to get a perfect looking elliptical arc
    // should it be choosen in function of the arc length anyway ?
    // or maybe a user settable parameter: toSBasis(unsigned int order) ?
    SBasis arc_x = ray(X) * cos(param,4);
    SBasis arc_y = ray(Y) * sin(param,4);
    arc[0] = arc_x * cos_rot_angle - arc_y * sin_rot_angle + Linear(center(X),center(X));
    arc[1] = arc_x * sin_rot_angle + arc_y * cos_rot_angle + Linear(center(Y),center(Y));
    return arc;
}


Coord SVGEllipticalArc::map_to_02PI(Coord t) const
{
    if ( sweep_flag() )
    {
        Coord angle = start_angle() + sweep_angle() * t;
        if ( !(angle < 2*M_PI) )
            angle -= 2*M_PI;
        return angle;
    }
    else
    {
        Coord angle = start_angle() - sweep_angle() * t;
        if ( angle < 0 ) angle += 2*M_PI;
        return angle;
    }
}

Coord SVGEllipticalArc::map_to_01(Coord angle) const
{
    return map_circular_arc_on_unit_interval(angle, start_angle(),
                                             end_angle(), sweep_flag());
}


} // end namespace Geom




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

