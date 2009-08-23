/**
 * \file
 * \brief Bezier curve
 *
 *//*
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Marco Cecchetti <mrcekets at gmail.com>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * 
 * Copyright 2007-2009 Authors
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

#include <2geom/bezier-curve.h>

namespace Geom 
{

BezierCurve *BezierCurve::optimize() const
{
    switch(order()) {
    case 1:
        if (!dynamic_cast<LineSegment const *>(this))
            return new LineSegment((*this)[0], (*this)[1]);
        break;
    case 2:
        if (!dynamic_cast<QuadraticBezier const *>(this))
            return new QuadraticBezier((*this)[0], (*this)[1], (*this)[2]);
        break;
    case 3:
        if (!dynamic_cast<CubicBezier const *>(this))
            return new CubicBezier((*this)[0], (*this)[1], (*this)[2], (*this)[3]);
        break;
    }
    return const_cast<BezierCurve*>(this);
}
Curve *BezierCurve::duplicate() const
{
    switch(order()) {
    case 1:
        return new LineSegment((*this)[0], (*this)[1]);
    case 2:
        return new QuadraticBezier((*this)[0], (*this)[1], (*this)[2]);
    case 3:
        return new CubicBezier((*this)[0], (*this)[1], (*this)[2], (*this)[3]);
    }
    return new BezierCurve(*this);
}

Curve *BezierCurve::derivative() const
{
    if (order() == 1) {
        double dx = inner[X][1] - inner[X][0], dy = inner[Y][1] - inner[Y][0];
        return new LineSegment(Point(dx,dy),Point(dx,dy));
    }
    return new BezierCurve(Geom::derivative(inner[X]), Geom::derivative(inner[Y]));
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
