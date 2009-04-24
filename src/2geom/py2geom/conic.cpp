/*
 * Copyright 2009 Nathan Hurst <njh@njhurst.com>
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
#include <boost/python/implicit.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include "py2geom.h"
#include "helpers.h"

#include "../point.h"
#include "../line.h"
#include "../conicsec.h"

using namespace boost::python;

// helpers for point
tuple xAx_to_tuple(Geom::xAx const& a)
{
    return make_tuple(a.c[0], a.c[1], a.c[2], a.c[3], a.c[4], a.c[5]);
}

Geom::xAx tuple_to_xAx(boost::python::tuple const& t)
{
    return Geom::xAx(extract<double>(t[0]),
                     extract<double>(t[1]),
                     extract<double>(t[2]),
                     extract<double>(t[3]),
                     extract<double>(t[4]),
                     extract<double>(t[5])
        );
}

std::vector<double> xax_roots1(Geom::xAx const & xax, Geom::Point const &a, Geom::Point const &b) { return xax.roots(a,b); }
std::vector<double> xax_roots2(Geom::xAx const & xax, Geom::Line const &l) { return xax.roots(l); }
Geom::SBasis homo_eval_at(Geom::xAx const & xax, 
                          Geom::SBasis const & x,
                          Geom::SBasis const & y,
                          Geom::SBasis const & w
    ) {
    return xax.evaluate_at(x, y, w);
}

Geom::SBasis xy_eval_at(Geom::xAx const & xax, 
                        Geom::SBasis const & x,
                        Geom::SBasis const & y
    ) {
    return xax.evaluate_at(x, y);
}

Geom::D2<Geom::SBasis> wrap_rq_to_cubic_sb(Geom::RatQuad const & rq) {
    return rq.toCubic().toSBasis();
}

Geom::D2<Geom::SBasis> wrap_rq_to_cubic_sb_l(Geom::RatQuad const & rq, double l) {
    return rq.toCubic(l).toSBasis();
}

std::vector<Geom::Point> wrap_rq_to_cubic_l(Geom::RatQuad const & rq, double l) {
    return  rq.toCubic(l).points();
}

std::vector<Geom::Point> wrap_rq_to_cubic(Geom::RatQuad const & rq) {
    return wrap_rq_to_cubic_l(rq, rq.lambda());
}

tuple wrap_rq_split(Geom::RatQuad const & rq) {
    Geom::RatQuad a, b;
    rq.split(a, b);
    return make_tuple(a, b);
}

object wrap_xax_to_curve(Geom::xAx const & xax, Geom::Rect const & r) {
    boost::optional<Geom::RatQuad> oc = xax.toCurve(r);
    return oc?object(*oc):object();
}



void wrap_conic() {
    //conicsec.h

    class_<Geom::xAx>("xAx", init<>())
        .def(init<double, double, double, double, double, double>())
        .def(init<Geom::xAx const&>())
        .def_readonly("c", &Geom::xAx::c)
        .def("tuple", xAx_to_tuple)
        
        .def("from_tuple", tuple_to_xAx)
        .staticmethod("from_tuple")
        .def("fromPoint", Geom::xAx::fromPoint)
        .staticmethod("fromPoint")
        .def("fromLine", (Geom::xAx (*)(Geom::Line l))Geom::xAx::fromLine)
        .staticmethod("fromLine")
        .def(self_ns::str(self))
        .def("valueAt", &Geom::xAx::valueAt)

        .def("implicit_form_coefficients", &Geom::xAx::implicit_form_coefficients)

        .def("isDegenerate", &Geom::xAx::isDegenerate)
        .def("roots", &xax_roots1)
        .def("roots", &xax_roots2)
        .def("extrema", &Geom::xAx::extrema)
        .def("gradient", &Geom::xAx::gradient)
        .def("evaluate_at", &xy_eval_at)
        .def("evaluate_at", &homo_eval_at)
        .def("toCurve", &wrap_xax_to_curve)
        .def(self - self)
        .def(self * float())
        ;

    class_<Geom::RatQuad>("RatQuad", init<>())
        .def(init<Geom::Point, Geom::Point, Geom::Point, double>())
        .def_readonly("P", &Geom::RatQuad::P)
        .def_readonly("w", &Geom::RatQuad::w)
        .def_readonly("lam", &Geom::RatQuad::lambda)
        //.def(self_ns::str(self))
        .def("pointAt", &Geom::RatQuad::pointAt)

        .def("toCubic", &wrap_rq_to_cubic)
        .def("toCubic", &wrap_rq_to_cubic_l)
        .def("toCubicSBasis", &wrap_rq_to_cubic_sb)
        .def("toCubicSBasis", &wrap_rq_to_cubic_sb_l)

        .def("split", &wrap_rq_split)
        .def("hermite", &Geom::RatQuad::hermite)
        .def("homogenous", &Geom::RatQuad::homogenous)
        ;
    implicitly_convertible<Geom::Point,tuple>();
};

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
