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

/**
 * @class BezierCurve
 * @brief Two-dimensional Bezier curve of arbitrary order.
 *
 * Bezier curves are an expansion of the concept of linear interpolation to n points, and adhere
 * to the definition of curves in the Curve class. Linear segments in 2Geom are in fact Bezier
 * curves of order 1.
 *
 * Let \f$\mathbf{B}_{\mathbf{p}_0\mathbf{p}_1\ldots\mathbf{p}_n}\f$ denote a Bezier curve
 * of order \f$n\f$ defined by the points \f$\mathbf{p}_0, \mathbf{p}_1, \ldots, \mathbf{p}_n\f$.
 * Bezier curve of order 1 is a linear interpolation curve between two points, defined as
 * \f[ \mathbf{B}_{\mathbf{p}_0\mathbf{p}_1}(t) = (1-t)\mathbf{p}_0 + t\mathbf{p}_1 \f]
 * If we now substitute points \f$\mathbf{p_0}\f$ and \f$\mathbf{p_1}\f$ in this definition
 * by linear interpolations, we get the definition of a Bezier curve of order 2, also called
 * a quadratic Bezier curve.
 * \f{align*}{ \mathbf{B}_{\mathbf{p}_0\mathbf{p}_1\mathbf{p}_2}(t)
       &= (1-t) \mathbf{B}_{\mathbf{p}_0\mathbf{p}_1}(t) + t \mathbf{B}_{\mathbf{p}_1\mathbf{p}_2}(t) \\
     \mathbf{B}_{\mathbf{p}_0\mathbf{p}_1\mathbf{p}_2}(t)
       &= (1-t)^2\mathbf{p}_0 + 2(1-t)t\mathbf{p}_1 + t^2\mathbf{p}_2 \f}
 * By substituting points for quadratic Bezier curves in the original definition,
 * we get a Bezier curve of order 3, called a cubic Bezier curve.
 * \f{align*}{ \mathbf{B}_{\mathbf{p}_0\mathbf{p}_1\mathbf{p}_2\mathbf{p}_3}(t)
       &= (1-t) \mathbf{B}_{\mathbf{p}_0\mathbf{p}_1\mathbf{p}_2}(t)
       + t \mathbf{B}_{\mathbf{p}_1\mathbf{p}_2\mathbf{p}_3}(t) \\
     \mathbf{B}_{\mathbf{p}_0\mathbf{p}_1\mathbf{p}_2\mathbf{p}_3}(t)
       &= (1-t)^3\mathbf{p}_0+3(1-t)^2t\mathbf{p}_1+3(1-t)t^2\mathbf{p}_2+t^3\mathbf{p}_3 \f}
 * In general, a Bezier curve or order \f$n\f$ can be recursively defined as
 * \f[ \mathbf{B}_{\mathbf{p}_0\mathbf{p}_1\ldots\mathbf{p}_n}(t)
     = (1-t) \mathbf{B}_{\mathbf{p}_0\mathbf{p}_1\ldots\mathbf{p}_{n-1}}(t)
     + t \mathbf{B}_{\mathbf{p}_1\mathbf{p}_2\ldots\mathbf{p}_n}(t) \f]
 *
 * This substitution can be repeated an arbitrary number of times. To picture this, imagine
 * the evaluation of a point on the curve as follows: first, all control points are joined with
 * straight lines, and a point corresponding to the sselected time value is marked on them.
 * Then, the marked points are joined with straight lines and the point corresponding to
 * the time value is marked. This is repeated until only one marked point remains, which is the
 * point at the selected time value.
 *
 * @image html bezier-curve-evaluation.png "Evaluation of the Bezier curve"
 *
 * An important property of the Bezier curves is that their parameters (control points)
 * have an intutive geometric interpretation. Because of this, they are frequently used
 * in vector graphics editors.
 *
 * Bezier curves of order 1, 2 and 3 are common enough to have their own more specific subclasses.
 * Note that if you create a generic BezierCurve, you cannot use a dynamic cast to those more
 * specific types, and you might face a slight preformance penalty relative to the more specific
 * classes. To obtain an instance of the correct optimized type, use the optimize()
 * or duplicate() methods. The difference is that optimize() will not create a new object
 * if it can't be optimized or is already an instance of one of the specific types, while
 * duplicate() will always create a new object.
 *
 * @ingroup Curves
 */

/** @brief Create an optimized instance of the curve.
     * If the curve was created as a generic Bezier curve but has an order between 1 and 3,
     * this method will create a newly allocated curve that is a LineSegment, a QuadraticBezier,
     * or a CubicBezier. For other cases it returns the pointer to the current curve, to avoid
     * allocating extra memory. Be careful when you use this method, because whether you have
     * to delete the returned object or not depends on its return value! If easier deletion
     * semantics are required, you can use the duplicate() method instead, which returns
     * optimized objects by default.
     * @return Pointer to a curve that is an instance of LineSegment, QuadraticBezier
     *         or CubicBezier based on the order. If the object is already an instance
     *         of te more specific types or the order is above 3, a pointer to the original
     *         object is returned instead, and no memory is allocated. */
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
