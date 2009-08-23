/**
 *  \file
 *  \brief Various trigoniometric helper functions
 *//*
 *  Authors:
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *   Marco Cecchetti <mrcekets at gmail.com>
 *
 * Copyright (C) 2007, 2008, 2009 authors
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
 *
 */

#ifndef LIB2GEOM_SEEN_ANGLE_H
#define LIB2GEOM_SEEN_ANGLE_H

#include <2geom/exception.h>
#include <2geom/coord.h>

#include <cmath>
#include <iostream>

namespace Geom {

#ifndef M_PI
# define M_PI 3.14159265358979323846
#endif

inline double deg_to_rad(double deg) { return deg*M_PI/180.0;}

inline double rad_to_deg(double rad) { return rad*180.0/M_PI;}

/*
 *  start_angle and angle must belong to [0, 2PI[
 *  and angle must belong to the cirsular arc defined by
 *  start_angle, end_angle and with rotation direction cw
 */
inline
double map_circular_arc_on_unit_interval( double angle, double start_angle, double end_angle, bool cw = true )
{
    double d = end_angle - start_angle;
    double t = angle - start_angle;
    if ( !cw )
    {
    	d = -d;
    	t = -t;
    }
    d = std::fmod(d, 2*M_PI);
    t = std::fmod(t, 2*M_PI);
    if ( d < 0 ) d += 2*M_PI;
    if ( t < 0 ) t += 2*M_PI;
    return t / d;
}

inline
Coord map_unit_interval_on_circular_arc(Coord t, double start_angle, double end_angle, bool cw = true)
{
	double sweep_angle = end_angle - start_angle;
	if ( !cw ) sweep_angle = -sweep_angle;
	sweep_angle = std::fmod(sweep_angle, 2*M_PI);
	if ( sweep_angle < 0 ) sweep_angle += 2*M_PI;

	Coord angle = start_angle;
    if ( cw )
    {
        angle += sweep_angle * t;
    }
    else
    {
        angle -= sweep_angle * t;
    }
    angle = std::fmod(angle, 2*M_PI);
    if (angle < 0) angle += 2*M_PI;
    return angle;
}

/*
 *  Return true if the given angle is contained in the circular arc determined
 *  by the passed angles.
 *
 *  a:     the angle to be tested
 *  sa:    the angle the arc start from
 *  ia:    an angle strictly inner to the arc
 *  ea:    the angle the arc end to
 *
 *  prerequisite: the inner angle has to be not equal (mod 2PI) to the start
 *                or the end angle, except when they are equal each other, too.
 *  warning: when sa == ea (mod 2PI) they define a whole circle
 *           if ia != sa (mod 2PI), on the contrary if ia == sa == ea (mod 2PI)
 *           they define a single point.
 */
inline
bool arc_contains (double a, double sa, double ia, double ea)
{
    a -= sa;
    a = std::fmod(a, 2*M_PI);
    if (a < 0) a += 2*M_PI;
    ia -= sa;
    ia = std::fmod(ia, 2*M_PI);
    if (ia < 0) ia += 2*M_PI;
    ea -= sa;
    ea = std::fmod(ea, 2*M_PI);
    if (ea < 0) ea += 2*M_PI;

    std::cout << "arc_contains: a = " << a << std::endl;
    std::cout << "arc_contains: ia = " << ia << std::endl;
    std::cout << "arc_contains: ea = " << ea << std::endl;
    if (ia == 0 && ea == 0)  return (a == 0);
    if (ia == 0 || ia == ea)
    {
        THROW_RANGEERROR ("arc_contains: passed angles do not define an arc");
    }
    return (ia < ea && a <= ea) || (ia > ea && (a >= ea || a == 0));
}

} // end namespace Geom

#endif


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
