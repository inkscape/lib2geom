/**
 * \file
 * \brief Symmetric power basis curve
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

#ifndef LIB2GEOM_SEEN_SBASIS_CURVE_H
#define LIB2GEOM_SEEN_SBASIS_CURVE_H

#include <2geom/curve.h>
#include <2geom/exception.h>
#include <2geom/nearest-time.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/transforms.h>

namespace Geom 
{

/** @brief Symmetric power basis curve.
 *
 * Symmetric power basis (S-basis for short) polynomials are a versatile numeric
 * representation of arbitrary continuous curves. They are the main representation of curves
 * in 2Geom.
 *
 * S-basis is defined for odd degrees and composed of the following polynomials:
 * \f{align*}{
         P_k^0(t) &= t^k (1-t)^{k+1} \\
         P_k^1(t) &= t^{k+1} (1-t)^k \f}
 * This can be understood more easily with the help of the chart below. Each square
 * represents a product of a specific number of \f$t\f$ and \f$(1-t)\f$ terms. Red dots
 * are the canonical (monomial) basis, the green dots are the Bezier basis, and the blue
 * dots are the S-basis, all of them of degree 7.
 *
 * @image html sbasis.png "Illustration of the monomial, Bezier and symmetric power bases"
 *
 * The S-Basis has several important properties:
 * - S-basis polynomials are closed under multiplication.
 * - Evaluation is fast, using a modified Horner scheme.
 * - Degree change is as trivial as in the monomial basis. To elevate, just add extra
 *   zero coefficients. To reduce the degree, truncate the terms in the highest powers.
 *   Compare this with Bezier curves, where degree change is complicated.
 * - Conversion between S-basis and Bezier basis is numerically stable.
 *
 * More in-depth information can be found in the following paper:
 * J Sanchez-Reyes, "The symmetric analogue of the polynomial power basis".
 * ACM Transactions on Graphics, Vol. 16, No. 3, July 1997, pages 319--357.
 * http://portal.acm.org/citation.cfm?id=256162
 *
 * @ingroup Curves
 */
class SBasisCurve : public Curve {
private:
    D2<SBasis> inner;
  
public:
    explicit SBasisCurve(D2<SBasis> const &sb) : inner(sb) {}
    explicit SBasisCurve(Curve const &other) : inner(other.toSBasis()) {}

    Curve *duplicate() const override { return new SBasisCurve(*this); }
    Point initialPoint() const override    { return inner.at0(); }
    Point finalPoint() const override      { return inner.at1(); }
    bool isDegenerate() const override     { return inner.isConstant(0); }
    bool isLineSegment() const override    { return inner[X].size() == 1; }
    Point pointAt(Coord t) const override  { return inner.valueAt(t); }
    std::vector<Point> pointAndDerivatives(Coord t, unsigned n) const override {
        return inner.valueAndDerivatives(t, n);
    }
    Coord valueAt(Coord t, Dim2 d) const override { return inner[d].valueAt(t); }
    void setInitial(Point const &v) override {
        for (unsigned d = 0; d < 2; d++) { inner[d][0][0] = v[d]; }
    }
    void setFinal(Point const &v) override {
        for (unsigned d = 0; d < 2; d++) { inner[d][0][1] = v[d]; }
    }
    Rect boundsFast() const override  { return *bounds_fast(inner); }
    Rect boundsExact() const override { return *bounds_exact(inner); }
    OptRect boundsLocal(OptInterval const &i, unsigned deg) const override {
        return bounds_local(inner, i, deg);
    }
    std::vector<Coord> roots(Coord v, Dim2 d) const override { return Geom::roots(inner[d] - v); }
    Coord nearestTime( Point const& p, Coord from = 0, Coord to = 1 ) const override {
        return nearest_time(p, inner, from, to);
    }
    std::vector<Coord> allNearestTimes( Point const& p, Coord from = 0,
        Coord to = 1 ) const override
    {
        return all_nearest_times(p, inner, from, to);
    }
    Coord length(Coord tolerance) const override { return ::Geom::length(inner, tolerance); }
    Curve *portion(Coord f, Coord t) const override {
        return new SBasisCurve(Geom::portion(inner, f, t));
    }

    using Curve::operator*=;
    void operator*=(Affine const &m) override { inner = inner * m; }

    Curve *derivative() const override {
        return new SBasisCurve(Geom::derivative(inner));
    }
    D2<SBasis> toSBasis() const override { return inner; }
    bool operator==(Curve const &c) const override {
        SBasisCurve const *other = dynamic_cast<SBasisCurve const *>(&c);
        if (!other) return false;
        return inner == other->inner;
    }
    bool isNear(Curve const &/*c*/, Coord /*eps*/) const override {
        THROW_NOTIMPLEMENTED();
        return false;
    }
    int degreesOfFreedom() const override {
        return inner[0].degreesOfFreedom() + inner[1].degreesOfFreedom();
    }
};

} // end namespace Geom

#endif // LIB2GEOM_SEEN_SBASIS_CURVE_H

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
