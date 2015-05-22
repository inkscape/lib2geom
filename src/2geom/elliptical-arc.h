/**
 * \file
 * \brief  Elliptical arc curve
 *
 *//*
 * Authors:
 *    MenTaLguY <mental@rydia.net>
 *    Marco Cecchetti <mrcekets at gmail.com>
 *    Krzysztof Kosiński <tweenk.pl@gmail.com>
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

#ifndef LIB2GEOM_SEEN_ELLIPTICAL_ARC_H
#define LIB2GEOM_SEEN_ELLIPTICAL_ARC_H

#include <algorithm>
#include <2geom/affine.h>
#include <2geom/angle.h>
#include <2geom/bezier-curve.h>
#include <2geom/curve.h>
#include <2geom/ellipse.h>
#include <2geom/sbasis-curve.h>  // for non-native methods
#include <2geom/utils.h>

namespace Geom 
{

class EllipticalArc : public Curve, public AngleInterval
{
public:
    /** @brief Creates an arc with all variables set to zero, and both flags to true. */
    EllipticalArc()
        : AngleInterval(0, 0, true)
        , _initial_point(0,0)
        , _final_point(0,0)
        , _large_arc(true)
    {}
    /** @brief Create a new elliptical arc.
     * @param ip Initial point of the arc
     * @param r Rays of the ellipse as a point
     * @param rot Angle of rotation of the X axis of the ellipse in radians
     * @param large If true, the large arc is chosen (always >= 180 degrees), otherwise
     *              the smaller arc is chosen
     * @param sweep If true, the clockwise arc is chosen, otherwise the counter-clockwise
     *              arc is chosen
     * @param fp Final point of the arc */
    EllipticalArc( Point const &ip, Point const &r,
                   Coord rot_angle, bool large_arc, bool sweep,
                   Point const &fp
                 )
        : AngleInterval(0,0,sweep)
        , _initial_point(ip)
        , _final_point(fp)
        , _ellipse(0, 0, r[X], r[Y], rot_angle)
        , _large_arc(large_arc)
    {
        _updateCenterAndAngles();
    }

    EllipticalArc( Point const &ip, Coord rx, Coord ry,
                   Coord rot_angle, bool large_arc, bool sweep,
                   Point const &fp
                 )
        : AngleInterval(0,0,sweep)
        , _initial_point(ip)
        , _final_point(fp)
        , _ellipse(0, 0, rx, ry, rot_angle)
        , _large_arc(large_arc)
    {
        _updateCenterAndAngles();
    }

    // methods new to EllipticalArc go here

    /// @name Retrieve and modify parameters
    /// @{
    /** @brief Get the interval of angles the arc contains
     * @return The interval between the final and initial angles of the arc */
    Interval angleInterval() const { return Interval(initialAngle(), finalAngle()); }
    /** @brief Get a coordinate of the elliptical arc's center.
     * @param d The dimension to retrieve
     * @return The selected coordinate of the center */
    /** @brief Get the defining ellipse's rotation
     * @return Angle between the +X ray of the ellipse and the +X axis */
    Angle rotationAngle() const {
        return _ellipse.rotationAngle();
    }
    /** @brief Get one of the ellipse's rays
     * @param d Dimension to retrieve
     * @return The selected ray of the ellipse */
    Coord ray(Dim2 d) const { return _ellipse.ray(d); }
    /** @brief Get both rays as a point
     * @return Point with X equal to the X ray and Y to Y ray */
    Point rays() const { return _ellipse.rays(); }
    /** @brief Whether the arc is larger than half an ellipse.
     * @return True if the arc is larger than \f$\pi\f$, false otherwise */
    bool largeArc() const { return _large_arc; }
    /** @brief Whether the arc turns clockwise
     * @return True if the arc makes a clockwise turn when going from initial to final
     *         point, false otherwise */
    bool sweep() const { return _sweep; }
    /** @brief Get the line segment connecting the arc's endpoints.
     * @return A linear segment with initial and final point correspoding to those of the arc. */
    LineSegment chord() const { return LineSegment(_initial_point, _final_point); }
    /** @brief Change the arc's parameters. */ 
    void set( Point const &ip, double rx, double ry,
              double rot_angle, bool large_arc, bool sweep,
              Point const &fp
            )
    {
        _initial_point = ip;
        _final_point = fp;
        _ellipse.setRays(rx, ry);
        _ellipse.setRotationAngle(rot_angle);
        _large_arc = large_arc;
        _sweep = sweep;
        _updateCenterAndAngles();
    }
    /** @brief Change the initial and final point in one operation.
     * This method exists because modifying any of the endpoints causes rather costly
     * recalculations of the center and extreme angles.
     * @param ip New initial point
     * @param fp New final point */
    void setEndpoints(Point const &ip, Point const &fp) {
        _initial_point = ip;
        _final_point = fp;
        _updateCenterAndAngles();
    }
    /// @}

    /// @name Access computed parameters of the arc
    /// @{
    Coord center(Dim2 d) const { return _ellipse.center(d); }
    /** @brief Get the arc's center
     * @return The arc's center, situated on the intersection of the ellipse's rays */
    Point center() const { return _ellipse.center(); }
    /// @}
    
    /// @name Angular evaluation
    /// @{
    /** Check whether the arc contains the given angle
     * @param t The angle to check
     * @return True if the arc contains the angle, false otherwise */
    bool containsAngle(Coord angle) const;
    /** @brief Evaluate the arc at the specified angular coordinate
     * @param t Angle
     * @return Point corresponding to the given angle */
    Point pointAtAngle(Coord t) const;
    /** @brief Evaluate one of the arc's coordinates at the specified angle
     * @param t Angle
     * @param d The dimension to retrieve
     * @return Selected coordinate of the arc at the specified angle */
    Coord valueAtAngle(Coord t, Dim2 d) const;
    /** @brief Retrieve the unit circle transform.
     * Each ellipse can be interpreted as a translated, scaled and rotate unit circle.
     * This function returns the transform that maps the unit circle to the arc's ellipse.
     * @return Transform from unit circle to the arc's ellipse */
    Affine unitCircleTransform() const;
    Affine inverseUnitCircleTransform() const;
    /// @}

    /** @brief Check whether both rays are nonzero.
     * If they are not, the arc is represented as a line segment instead. */
    bool isChord() const {
        return ray(X) == 0 || ray(Y) == 0;
    }

    std::pair<EllipticalArc, EllipticalArc> subdivide(Coord t) const {
        EllipticalArc* arc1 = static_cast<EllipticalArc*>(portion(0, t));
        EllipticalArc* arc2 = static_cast<EllipticalArc*>(portion(t, 1));
        assert( arc1 != NULL && arc2 != NULL);
        std::pair<EllipticalArc, EllipticalArc> arc_pair(*arc1, *arc2);        
        delete arc1;
        delete arc2;
        return arc_pair;
    }

    // implementation of overloads goes here
    virtual Point initialPoint() const { return _initial_point; }
    virtual Point finalPoint() const { return _final_point; }
    virtual Curve* duplicate() const { return new EllipticalArc(*this); }
    virtual void setInitial(Point const &p) {
        _initial_point = p;
        _updateCenterAndAngles();
    }
    virtual void setFinal(Point const &p) {
        _final_point = p;
        _updateCenterAndAngles();
    }
    virtual bool isDegenerate() const {
        return _initial_point == _final_point;
    }
    virtual bool isLineSegment() const { return isChord(); }
    virtual Rect boundsFast() const {
        return boundsExact();
    }
    virtual Rect boundsExact() const;
    // TODO: native implementation of the following methods
    virtual OptRect boundsLocal(OptInterval const &i, unsigned int deg) const {
        return SBasisCurve(toSBasis()).boundsLocal(i, deg);
    }
    virtual std::vector<double> roots(double v, Dim2 d) const;
#ifdef HAVE_GSL
    virtual std::vector<double> allNearestTimes( Point const& p, double from = 0, double to = 1 ) const;
#endif
    virtual double nearestTime( Point const& p, double from = 0, double to = 1 ) const {
        if ( are_near(ray(X), ray(Y)) && are_near(center(), p) ) {
            return from;
        }
        return allNearestTimes(p, from, to).front();
    }
    virtual std::vector<CurveIntersection> intersect(Curve const &other, Coord eps=EPSILON) const;
    virtual int degreesOfFreedom() const { return 7; }
    virtual Curve *derivative() const;
    virtual void transform(Affine const &m);
    virtual Curve &operator*=(Translate const &m) {
        _initial_point *= m;
        _final_point *= m;
        _ellipse *= m;
        return *this;
    }


    /**
    *  The size of the returned vector equals n+1.
    */
    virtual std::vector<Point> pointAndDerivatives(Coord t, unsigned int n) const;

    virtual D2<SBasis> toSBasis() const;
    virtual double valueAt(Coord t, Dim2 d) const {
        if (isChord()) return chord().valueAt(t, d);
        return valueAtAngle(angleAt(t), d);
    }
    virtual Point pointAt(Coord t) const;
    virtual Curve* portion(double f, double t) const;
    virtual Curve* reverse() const;
    virtual bool operator==(Curve const &c) const;
    virtual void feed(PathSink &sink, bool moveto_initial) const;

protected:
    void _updateCenterAndAngles();
    void _filterIntersections(std::vector<ShapeIntersection> &xs, bool is_first) const;

    Point _initial_point, _final_point;
    Ellipse _ellipse;
    bool _large_arc;

private:
    Coord map_to_01(Coord angle) const; 
}; // end class EllipticalArc


// implemented in elliptical-arc-from-sbasis.cpp
bool arc_from_sbasis(EllipticalArc &ea, D2<SBasis> const &in,
                     double tolerance = EPSILON, unsigned num_samples = 20);

std::ostream &operator<<(std::ostream &out, EllipticalArc const &ea);

} // end namespace Geom

#endif // LIB2GEOM_SEEN_ELLIPTICAL_ARC_H

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
