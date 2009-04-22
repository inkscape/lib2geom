/**
 * \file
 * \brief Conic Section
 *
 * Authors:
 *      Nathan Hurst <njh@njhurst.com>
 *
 * Copyright 2009  authors
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


#ifndef _2GEOM_CONIC_SECTION_H_
#define _2GEOM_CONIC_SECTION_H_

#include <2geom/point.h>
#include <2geom/exception.h>
#include <2geom/bezier-curve.h>


namespace Geom
{

class RatQuad{
    /**
     * A curve of the form B02*A + B12*B*w + B22*C/(B02 + B12*w + B22)
     * These curves can exactly represent a piece conic section less than a certain angle (find out)
     * 
     **/

public:
    Point P[3];
    double w;
    RatQuad() {}
    RatQuad(Point a, Point b, Point c, double w) : w(w) {
        P[0] = a;
        P[1] = b;
        P[2] = c;
    }
    double lambda() const;
  
    static RatQuad fromPointsTangents(Point P0, Point dP0,
                                      Point P,
                                      Point P2, Point dP2);
  
    CubicBezier toCubic() const;

    Point pointAt(double t);
  
    void split(RatQuad &a, RatQuad &b) const;

    D2<SBasis> hermite();
    std::vector<SBasis> homogenous();
};

class xAx{
public:
    double c[6];

    xAx(double c0, double c1, double c2, double c3, double c4, double c5) {
        c[0] = c0;   c[1] = c1;   c[2] = c2; // xx, xy, yy
        c[3] = c3;   c[4] = c4; // x, y
        c[5] = c5; // 1
    }
    xAx() {}
    std::string categorise() const;
    bool isDegenerate() const;
    static xAx fromPoint(Point p);
    static xAx fromDistPoint(Point p, double d);
    static xAx fromLine(Point n, double d);
    static xAx fromLine(Line l);

    template<typename T>
    T evaluate_at(T x, T y) const {
        return c[0]*x*x + c[1]*x*y + c[2]*y*y + c[3]*x + c[4]*y + c[5];
    }

    double valueAt(Point P);
    
    std::vector<double> implicit_form_coefficients() {
        return std::vector<double>(c, c+6);
    }

    template<typename T>
    T evaluate_at(T x, T y, T w) const {
        return c[0]*x*x + c[1]*x*y + c[2]*y*y + c[3]*x*w + c[4]*y*w + c[5]*w*w;
    }

    xAx scale(double sx, double sy);

    Point gradient(Point p);
  
    xAx operator-(xAx const &b) const;
    xAx operator+(double const &b) const;
    xAx operator*(double const &b) const;
    
    std::vector<Point> crossings(Rect r);
    boost::optional<RatQuad> toCurve(Rect const & bnd);
    std::vector<double> roots(Point d, Point o) const;

    std::vector<double> roots(Line const &l) const;
  
    static Interval quad_ex(double a, double b, double c, Interval ivl);
    
    Geom::Matrix hessian() const;
    
    Point bottom() const;
    
    Interval extrema(Rect r);
};

inline std::ostream &operator<< (std::ostream &out_file, const xAx &x) {
    for(int i = 0; i < 6; i++) {
        out_file << x.c[i] << ", ";
    }
    return out_file;
}

};


#endif // _2GEOM_CONIC_SECTION_H_


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

