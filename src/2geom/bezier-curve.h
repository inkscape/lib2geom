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

#ifndef _2GEOM_BEZIER_CURVE_H_
#define _2GEOM_BEZIER_CURVE_H_

#include <2geom/curve.h>
#include <2geom/sbasis-curve.h> // for non-native winding method
#include <2geom/bezier.h>

namespace Geom 
{

class BezierCurve : public Curve {
public:
    /// @name Construct the curve
    /// @{
    /** @brief Construct a Bezier curve of the specified order with all points zero. */
    explicit BezierCurve(unsigned order) : inner(Bezier::Order(order), Bezier::Order(order)) {}
    /** @brief Construct from 2D Bezier polynomial. */
    explicit BezierCurve(D2<Bezier > const &x) : inner(x) {
        if (inner[X].order() != inner[Y].order())
            THROW_LOGICALERROR("Beziers used to construct a BezierCurve "
                               "are not of the same order");
    }
    /** @brief Construct from two 1D Bezier polynomials of the same order. */
    BezierCurve(Bezier x, Bezier y) : inner(x, y) {
        // throw an error if the orders do not match
        if (x.order() != y.order())
            THROW_LOGICALERROR("Beziers used to construct a BezierCurve "
                               "are not of the same order");
    }
    /** @brief Construct a linear segment from its endpoints. */
    BezierCurve(Point c0, Point c1) {
        for(unsigned d = 0; d < 2; d++)
            inner[d] = Bezier(c0[d], c1[d]);
    }
    /** @brief Construct a quadratic Bezier curve from its control points. */
    BezierCurve(Point c0, Point c1, Point c2) {
        for(unsigned d = 0; d < 2; d++)
            inner[d] = Bezier(c0[d], c1[d], c2[d]);
    }
    /** @brief Construct a cubic Bezier curve from its control points. */
    BezierCurve(Point c0, Point c1, Point c2, Point c3) {
        for(unsigned d = 0; d < 2; d++)
            inner[d] = Bezier(c0[d], c1[d], c2[d], c3[d]);
    }
    /** @brief Construct a Bezier curve from a vector of its control points. */
    BezierCurve(std::vector<Point> const &points) {
        unsigned ord = points.size() - 1;
        if (ord < 1) THROW_LOGICALERROR("Not enough points to construct a Bezier curve");
        for (unsigned d = 0; d < 2; ++d) {
            inner[d] = Bezier(Bezier::Order(ord));
            for(unsigned i = 0; i <= ord; i++)
                inner[d][i] = points[i][d];
        }
    }
    /// @}

    // methods new to BezierCurve go here

    /// @name Access and modify control points
    /// @{
    /** @brief Get the order of the Bezier curve.
     * A Bezier curve has order() + 1 control points. */
    unsigned order() const { return inner[X].order(); }
    /** @brief Get the control points.
     * @return Vector with order() + 1 control points. */
    std::vector<Point> points() const { return bezier_points(inner); }
    /** @brief Modify a control point.
     * @param ix The zero-based index of the point to modify
     * @param v The new value of the point */
    void setPoint(unsigned ix, Point v) {
        inner[X].setPoint(ix, v[X]);
        inner[Y].setPoint(ix, v[Y]);
    }
    /** @brief Set new control points for this curve.
     * @param ps Vector which must contain order() + 1 points. */
    virtual void setPoints(std::vector<Point> const &ps) {
        // must be virtual, because HLineSegment will need to redefine it
        if (ps.size() != order() + 1)
            THROW_LOGICALERROR("BezierCurve::setPoints: incorrect number of points in vector");
        for(unsigned i = 0; i <= order(); i++) {
            setPoint(i, ps[i]);
        }
    }
    /** Access control points of the curve.
     * @param ix Index of the control point
     * @return The control point. No-reference return, use setPoint() to modify control points. */
    Point const operator[](unsigned ix) const { return Point(inner[X][ix], inner[Y][ix]); }
    /// @}

    /** @brief Divide a Bezier curve into two curves
     * @param t Time value
     * @return Pair of Bezier curves \f$(\mathbf{D}, \mathbf{E})\f$ such that
     *         \f$\mathbf{D}[ [0,1] ] = \mathbf{C}[ [0,t] ]\f$ and
     *         \f$\mathbf{E}[ [0,1] ] = \mathbf{C}[ [t,1] ]\f$ */
    std::pair<BezierCurve, BezierCurve> subdivide(Coord t) const {
        std::pair<Bezier, Bezier> sx = inner[X].subdivide(t), sy = inner[Y].subdivide(t);
        return std::make_pair(
                   BezierCurve(sx.first, sy.first),
                   BezierCurve(sx.second, sy.second));
    }
    BezierCurve *optimize() const;

    // implementation of virtual methods goes here
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    virtual Curve *duplicate() const;
    virtual Point initialPoint() const { return inner.at0(); }
    virtual Point finalPoint() const { return inner.at1(); }
    virtual bool isDegenerate() const { return inner.isConstant(); }
    virtual void setInitial(Point const &v) { setPoint(0, v); }
    virtual void setFinal(Point const &v) { setPoint(order(), v); }
    virtual Rect boundsFast() const { return *bounds_fast(inner); }
    virtual Rect boundsExact() const { return *bounds_exact(inner); }
    virtual OptRect boundsLocal(OptInterval const &i, unsigned deg) const {
        if (!i) return OptRect();
        if(i->min() == 0 && i->max() == 1) return boundsFast();
        if(deg == 0) return bounds_local(inner, i);
        // TODO: UUUUUUGGGLLY
        if(deg == 1 && order() > 1) return OptRect(bounds_local(Geom::derivative(inner[X]), i),
                                                   bounds_local(Geom::derivative(inner[Y]), i));
        return OptRect();
    }
    virtual int degreesOfFreedom() const {
        return 2 * inner[X].order();
    }
    virtual std::vector<Coord> roots(Coord v, Dim2 d) const {
        return (inner[d] - v).roots();
    }
    virtual Coord nearestPoint(Point const &p, Coord from = 0, Coord to = 1 ) const {
        return Curve::nearestPoint(p, from, to);
    }
    Curve *portion(Coord f, Coord t) const {
        return new BezierCurve(Geom::portion(inner, f, t));
    }
    Curve *reverse() const {
        return new BezierCurve(Geom::reverse(inner));
    }
    virtual Curve *transformed(Affine const &m) const {
        BezierCurve *ret = new BezierCurve(order());
        std::vector<Point> ps = points();
        for(unsigned i = 0;  i <= order(); i++) ps[i] = ps[i] * m;
        ret->setPoints(ps);
        return ret;
    }
    virtual Curve *derivative() const;
    virtual Point pointAt(Coord t) const { return inner.valueAt(t); }
    virtual std::vector<Point> pointAndDerivatives(Coord t, unsigned n) const { return inner.valueAndDerivatives(t, n); }
    virtual Coord valueAt(Coord t, Dim2 d) const { return inner[d].valueAt(t); }
    virtual D2<SBasis> toSBasis() const {return inner.toSBasis(); }
#endif

protected:
    /*BezierCurve(Point c[]) {
        Coord *x = new Coord[order()+1];
        Coord *y = new Coord[order()+1];
        for(unsigned i = 0; i <= order(); i++) {
            x[i] = c[i][X]; y[i] = c[i][Y];
        }
        inner[X] = Bezier(x, order());
        inner[Y] = Bezier(y, order());
    }*/

    D2<Bezier> inner;
};


/** @brief Linear segment, a special case of a Bezier curve.
 *
 * This class uses some optimizations for a linear segment (a Bezier curve of order 1).
 * Note that if you created a BezierCurve, you will not be able to cast it to a LineSegment
 * using a dynamic cast regardless of its order - use the optimize() method.
 *
 * @ingroup Curves */
class LineSegment : public BezierCurve {
public:
    LineSegment() : BezierCurve(1) {}
    LineSegment(Point c0, Point c1) : BezierCurve(c0, c1) {}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
    virtual Coord nearestPoint(Point const& p, Coord from = 0, Coord to = 1) const {
        if ( from > to ) std::swap(from, to);
        Point ip = pointAt(from);
        Point fp = pointAt(to);
        Point v = fp - ip;
        Coord l2v = L2sq(v);
        if(l2v == 0) return 0;
        Coord t = dot( p - ip, v ) / l2v;
        if ( t <= 0 )  		return from;
        else if ( t >= 1 )  return to;
        else return from + t*(to-from);
    }
    virtual Curve *derivative() const {
        Coord dx = inner[X][1] - inner[X][0], dy = inner[Y][1] - inner[Y][0];
        return new LineSegment(Point(dx,dy),Point(dx,dy));
    }
#endif
};

/** @brief Quadratic Bezier curve.
 * Note that if you created a BezierCurve, you will not be able to cast it to a QuadraticBezier
 * using a dynamic cast regardless of its order - use the optimize() method.
 * @ingroup Curves */
class QuadraticBezier : public BezierCurve {
public:
    QuadraticBezier(Point c0, Point c1, Point c2) : BezierCurve(c0, c1, c2) {}
};

/** @brief Cubic Bezier curve.
 * Note that if you created a BezierCurve, you will not be able to cast it to a CubicBezier
 * using a dynamic cast regardless of its order - use the optimize() method.
 * @ingroup Curves */
class CubicBezier : public BezierCurve {
public:
    CubicBezier(Point c0, Point c1, Point c2, Point c3) : BezierCurve(c0, c1, c2, c3) {}
};

inline Point middle_point(LineSegment const& _segment) {
    return ( _segment.initialPoint() + _segment.finalPoint() ) / 2;
}

inline Coord length(LineSegment const& seg) {
    return distance(seg.initialPoint(), seg.finalPoint());
}


} // end namespace Geom

#endif // _2GEOM_BEZIER_CURVE_H_

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
