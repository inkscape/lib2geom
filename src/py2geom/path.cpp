/*
 * Python bindings for lib2geom
 *
 * Copyright 2007 Aaron Spike <aaron@ekips.org>
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

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include "2geom/curve.h"
#include "2geom/bezier-curve.h"
#include "2geom/path.h"
#include "2geom/pathvector.h"
#include "2geom/sbasis-to-bezier.h"
#include "helpers.h"

#include "2geom/point.h"
#include "2geom/rect.h"
#include "2geom/d2.h"

using namespace boost::python;

Geom::Curve const &path_getitem(Geom::Path const& p, int index)
{
    unsigned size = p.size_default();
    unsigned i = index;
    if (index < 0)
    {
        i = index = size + index;
    }
    if ((index < 0) || (i > (size - 1))) {
        PyErr_SetString(PyExc_IndexError, "index out of range");
        boost::python::throw_error_already_set();
    }
    return p[i];
}

struct CurveWrap : Geom::Curve, wrapper<Geom::Curve>
{
    Geom::Point initialPoint() const {return this->get_override("initialPoint")();}
    Geom::Point finalPoint() const {return this->get_override("finalPoint")();}
    bool isDegenerate() const {return this->get_override("isDegenerate")();}
    CurveWrap *duplicate() const {return this->get_override("duplicate")();}
    Geom::Rect boundsFast() const {return this->get_override("boundsFast")();}
    Geom::Rect boundsExact() const {return this->get_override("boundsExact")();}
    virtual Geom::OptRect boundsLocal(Geom::OptInterval const &i, unsigned deg) const {return this->get_override("boundsLocal")(i,deg);}
    std::vector<double> roots(double v, Geom::Dim2 d) const {return this->get_override("roots")(v,d);}

    int winding(Geom::Point const &p) const {
        if (override f = this->get_override("winding")) {
            return f(p);
        }
        return Geom::Curve::winding(p);
    }
    int default_winding(Geom::Point p) const { return this->Geom::Curve::winding(p); }

    Geom::Curve *portion(double f, double t) const { return this->get_override("portion")(f,t); }
    Geom::Curve *reverse() const { 
        if (override f = this->get_override("reverse")) {
            return f();
        }
        return Geom::Curve::reverse(); 
    }
    Geom::Curve *default_reverse() const { return this->Geom::Curve::reverse(); }

    Geom::Curve *derivative() const { return this->get_override("derivative")(); }


    Geom::Curve *transformed(Geom::Affine const &m) const { return this->get_override("transformed")(m); }

    Geom::Point pointAt(Geom::Coord t) const {
        if (override f = this->get_override("pointAt")) {
            return f(t);
        }
        return Geom::Curve::pointAt(t);
    }
    Geom::Point default_pointAt(Geom::Coord t) { return this->Geom::Curve::pointAt(t); }
    std::vector<Geom::Point> pointAndDerivatives(Geom::Coord t, unsigned n) const {
        return this->get_override("pointAndDerivatives")(t, n);
    }

    Geom::D2<Geom::SBasis> toSBasis() const {return this->get_override("sbasis")();}
};


/* pycairo stuff: */
#ifdef HAVE_PYCAIRO

#include "cairo-helpers.h"

void py_cairo_curve(object cr, Geom::Curve const &c) {
    cairo_curve(cairo_t_from_object(cr), c);
}
void py_cairo_rectangle(object cr, Geom::Rect const &r) {
    cairo_rectangle(cairo_t_from_object(cr), r);
}

void py_cairo_convex_hull(object cr, Geom::ConvexHull const &r) {
    cairo_convex_hull(cairo_t_from_object(cr), r);
}
/*void py_cairo_path(object cr, Geom::Path const &p) {
    cairo_path(cairo_t_from_object(cr), p);
    }*/

void py_cairo_path(object cr, Geom::Path const &p) {
    cairo_path(cairo_t_from_object(cr), p);
}

void py_cairo_path(object cr, Geom::PathVector const &p) {
    cairo_path(cairo_t_from_object(cr), p);
}
void py_cairo_path_stitches(object cr, Geom::Path const &p) {
    cairo_path_stitches(cairo_t_from_object(cr), p);
}
void py_cairo_path_stitches(object cr, Geom::PathVector const &p) {
    cairo_path_stitches(cairo_t_from_object(cr), p);
}
void     (*cp_1)(object, Geom::Path const &)    = &py_cairo_path;
void     (*cp_2)(object, Geom::PathVector const &)    = &py_cairo_path;

void     (*cps_1)(object, Geom::Path const &)    = &py_cairo_path_stitches;
void     (*cps_2)(object, Geom::PathVector const &)    = &py_cairo_path_stitches;


void py_cairo_d2_sb(object cr, Geom::D2<Geom::SBasis> const &p) {
    cairo_d2_sb(cairo_t_from_object(cr), p);
}

void py_cairo_d2_pw_sb(object cr, Geom::D2<Geom::Piecewise<Geom::SBasis> > const &p) {
    cairo_d2_pw_sb(cairo_t_from_object(cr), p);
}

void py_cairo_pw_d2_sb(object cr, Geom::Piecewise<Geom::D2<Geom::SBasis> > const &p) {
    cairo_pw_d2_sb(cairo_t_from_object(cr), p);
}

#endif // HAVE_PYCAIRO

Geom::Point (Geom::Path::*path_pointAt_time)(Geom::Coord) const = &Geom::Path::pointAt;
Geom::Coord (Geom::Path::*path_valueAt_time)(Geom::Coord, Geom::Dim2) const = &Geom::Path::valueAt;
void (Geom::Path::*appendPortionTo_time)(Geom::Path &, Geom::Coord, Geom::Coord) const = &Geom::Path::appendPortionTo;
//void (Geom::Path::*appendPortionTo_pos)(Geom::Path &, Geom::PathPosition const &, Geom::PathPosition const &, bool) const = &Geom::Path::appendPortionTo;

void wrap_path()
{
/*    class_<CurveWrap, boost::noncopyable>("Curve")
        .def("initalPoint", pure_virtual(&Geom::Curve::initialPoint))
        .def("finalPoint", pure_virtual(&Geom::Curve::finalPoint))
        .def("duplicate", pure_virtual(&Geom::Curve::duplicate), return_value_policy<manage_new_object>())
        .def("boundsFast", pure_virtual(&Geom::Curve::boundsFast))
        .def("boundsExact", pure_virtual(&Geom::Curve::boundsExact))
        //.def("pointAt", &Geom::Curve::pointAt, &CurveWrap::default_pointAt)
        //.def("winding", &Geom::Curve::winding, &CurveWrap::default_winding)
        .def("pointAndDerivatives", pure_virtual(&Geom::Curve::pointAndDerivatives))
        .def("toSBasis", pure_virtual(&Geom::Curve::toSBasis))
        ;*/
/*    class_<Geom::LineSegment, bases<CurveWrap> >("LineSegment")
        .def("points", &Geom::LineSegment::points)
    ;
    class_<Geom::QuadraticBezier, bases<CurveWrap> >("QuadraticBezier")
        .def("points", &Geom::QuadraticBezier::points)
    ;
    class_<Geom::CubicBezier, bases<CurveWrap> >("CubicBezier")
        .def("points", &Geom::CubicBezier::points)
        ;*/
    class_<Geom::Path>("Path")
        .def("__getitem__", path_getitem, return_value_policy<copy_const_reference>()) //or return_internal_reference see http://www.boost.org/doc/libs/1_36_0/libs/python/doc/v2/faq.html#question1
        .def("empty", &Geom::Path::empty)
        .def("closed", &Geom::Path::closed)
        .def("close", &Geom::Path::close)
        .def("boundsFast", &Geom::Path::boundsFast)
        .def("boundsExact", &Geom::Path::boundsExact)
        .def("toPwSb", &Geom::Path::toPwSb)
        .def(self * Geom::Affine())
        .def(self *= Geom::Affine())
        .def("pointAt", path_pointAt_time)
        .def("valueAt", path_valueAt_time)
        .def("__call__", path_pointAt_time)
        .def("roots", &Geom::Path::roots)
        //.def("allNearestTimes", &Geom::Path::allNearestTimes)
        //.def("nearestTime", &Geom::Path::nearestTime)
        .def("appendPortionTo", appendPortionTo_time)
        //.def("portion", &Geom::Path::portion)
        .def("reversed", &Geom::Path::reversed)
        //.def("insert", &Geom::Path::insert)
        .def("clear", &Geom::Path::clear)
        //.def("erase", &Geom::Path::erase)
        .def("erase_last", &Geom::Path::erase_last)
        //.def("replace", &Geom::Path::replace)
        .def("start", &Geom::Path::start)
        .def("initialPoint", &Geom::Path::initialPoint)
        .def("finalPoint", &Geom::Path::finalPoint)
        //.def("append", &Geom::Path::append)
        //.def("appendNew", &Geom::Path::appendNew)
    ;
    def("paths_to_pw",Geom::paths_to_pw);
    class_<Geom::PathVector >("PathVector")
        .def(vector_indexing_suite<Geom::PathVector >())
        .def(self * Geom::Affine())
        .def(self *= Geom::Affine())
        .def("reversed", &Geom::PathVector::reversed)
        .def("reverse", &Geom::PathVector::reverse)
        .def("boundsFast", &Geom::PathVector::boundsFast)
        .def("boundsExact", &Geom::PathVector::boundsExact)
    ;
    def("path_from_piecewise", Geom::path_from_piecewise);
    def("path_from_sbasis", Geom::path_from_sbasis);
    def("cubicbezierpath_from_sbasis", Geom::cubicbezierpath_from_sbasis);

#ifdef HAVE_PYCAIRO
void cairo_move_to(cairo_t *cr, Geom::Point p1);
    def("cubicbezierpath_from_sbasis", Geom::cubicbezierpath_from_sbasis);
void cairo_line_to(cairo_t *cr, Geom::Point p1);
    def("cubicbezierpath_from_sbasis", Geom::cubicbezierpath_from_sbasis);
void cairo_curve_to(cairo_t *cr, Geom::Point p1, Geom::Point p2, Geom::Point p3);
    def("cubicbezierpath_from_sbasis", Geom::cubicbezierpath_from_sbasis);

    //def("cairo_curve", cairo_curve);
    def("cairo_convex_hull", py_cairo_convex_hull);
    def("cairo_path", cp_1);
    def("cairo_path", cp_2);
    def("cairo_path_stitches", cps_1);
    def("cairo_path_stitches", cps_2);

    def("cairo_d2_sb", py_cairo_d2_sb);
    def("cairo_d2_pw_sb", py_cairo_d2_pw_sb);
    def("cairo_pw_d2_sb", py_cairo_pw_d2_sb);
#endif // HAVE_PYCAIRO
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(substatement-open . 0))
  indent-tabs-mode:nil
  c-brace-offset:0
  fill-column:99
  End:
  vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/
