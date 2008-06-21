/*
 * Copyright 2006, 2007 Aaron Spike <aaron@ekips.org>
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

#include "../sbasis.h"
#include "../piecewise.h"
#include "../d2.h"
#include "../sbasis-math.h"

#include "py2geom.h"
#include "helpers.h"

using namespace boost::python;

Geom::Piecewise<Geom::SBasis> (*portion_pwsb)(const Geom::Piecewise<Geom::SBasis> &, double, double) = &Geom::portion;
std::vector<double> (*roots_pwsb)(const Geom::Piecewise<Geom::SBasis> &) = &Geom::roots;
//Geom::Piecewise<Geom::SBasis> (*multiply_pwsb)(Geom::Piecewise<Geom::SBasis> const &, Geom::Piecewise<Geom::SBasis> const &) = &Geom::multiply;
Geom::Piecewise<Geom::SBasis> (*divide_pwsb)(Geom::Piecewise<Geom::SBasis> const &, Geom::Piecewise<Geom::SBasis> const &, unsigned) = &Geom::divide;
Geom::Piecewise<Geom::SBasis> (*compose_pwsb_sb)(Geom::Piecewise<Geom::SBasis> const &, Geom::SBasis const &) = &Geom::compose;
Geom::Piecewise<Geom::SBasis> (*compose_pwsb)(Geom::Piecewise<Geom::SBasis> const &, Geom::Piecewise<Geom::SBasis> const &) = &Geom::compose;

Geom::Piecewise<Geom::SBasis> (*abs_pwsb)(Geom::Piecewise<Geom::SBasis> const &) = &Geom::abs;

Geom::Piecewise<Geom::SBasis> (*min_pwsb)(Geom::Piecewise<Geom::SBasis> const &, Geom::Piecewise<Geom::SBasis> const &) = &Geom::min;
Geom::Piecewise<Geom::SBasis> (*max_pwsb)(Geom::Piecewise<Geom::SBasis> const &, Geom::Piecewise<Geom::SBasis> const &) = &Geom::max;
Geom::Piecewise<Geom::SBasis> (*signSb_pwsb)(Geom::Piecewise<Geom::SBasis> const &) = &Geom::signSb;

Geom::Piecewise<Geom::SBasis> (*sqrt_pwsb)(Geom::Piecewise<Geom::SBasis> const &, double, int) = &Geom::sqrt;

Geom::Piecewise<Geom::SBasis> (*sin_pwsb)(Geom::Piecewise<Geom::SBasis> const &, double, int) = &Geom::sin;
Geom::Piecewise<Geom::SBasis> (*cos_pwsb)(Geom::Piecewise<Geom::SBasis> const &, double, int) = &Geom::cos;

//Geom::Piecewise<Geom::SBasis> (*log_pwsb)(Geom::Piecewise<Geom::SBasis> const &, double, int) = &Geom::log;
Geom::Piecewise<Geom::SBasis> (*reciprocal_pwsb)(Geom::Piecewise<Geom::SBasis> const &, double, int) = &Geom::reciprocal;

Geom::FragmentConcept<Geom::SBasis>::BoundsType (*bounds_fast_pwsb)(Geom::Piecewise<Geom::SBasis> const &) = &Geom::bounds_fast;
Geom::FragmentConcept<Geom::SBasis>::BoundsType (*bounds_exact_pwsb)(Geom::Piecewise<Geom::SBasis> const &) = &Geom::bounds_exact;
Geom::FragmentConcept<Geom::SBasis>::BoundsType (*bounds_local_pwsb)(Geom::Piecewise<Geom::SBasis> const &, const Geom::Interval &) = &Geom::bounds_local;

Geom::SBasis getitem_pwsb(Geom::Piecewise<Geom::SBasis> const &p, unsigned const index) {
    unsigned D = p.size();
    unsigned i = index;
    if (index < 0)
    {
        i = D + index;
    }
    if (i < 0 || i > (D - 1)) {
        PyErr_SetString(PyExc_IndexError, "index out of range");
        boost::python::throw_error_already_set();
    }
    return p[i];
}

void wrap_pw() {
    class_<std::vector<Geom::SBasis> >("SBasisVec")
        .def(vector_indexing_suite<std::vector<Geom::SBasis> >())
    ;

    def("portion", portion_pwsb);
    //def("partition", &partition);
    def("roots", roots_pwsb);
    //def("multiply", multiply_pwsb);
    def("divide", divide_pwsb);
    def("compose", compose_pwsb_sb);
    def("compose", compose_pwsb);
    def("abs", abs_pwsb);
    def("min", min_pwsb);
    def("max", max_pwsb);
    def("signSb", signSb_pwsb);
    def("sqrt", sqrt_pwsb);
    def("cos", cos_pwsb);
    def("sin", sin_pwsb);
    //def("log", log_pwsb);
    def("reciprocal", reciprocal_pwsb);
    def("bounds_fast", bounds_fast_pwsb);
    def("bounds_exact", bounds_exact_pwsb);
    def("bounds_local", bounds_local_pwsb);

    class_<Geom::Piecewise<Geom::SBasis> >("PiecewiseSBasis")
        .def("__getitem__", getitem_pwsb)
        .def("__call__", &Geom::Piecewise<Geom::SBasis>::valueAt)
        .def_readonly("cuts", &Geom::Piecewise<Geom::SBasis>::cuts)
        .def_readonly("segs", &Geom::Piecewise<Geom::SBasis>::segs)
        .def("valueAt", &Geom::Piecewise<Geom::SBasis>::valueAt)
        .def("size", &Geom::Piecewise<Geom::SBasis>::size)
        .def("empty", &Geom::Piecewise<Geom::SBasis>::empty)
        .def("push", &Geom::Piecewise<Geom::SBasis>::push)
        .def("push_cut", &Geom::Piecewise<Geom::SBasis>::push_cut)
        .def("push_seg", &Geom::Piecewise<Geom::SBasis>::push_seg)

        .def("segN", &Geom::Piecewise<Geom::SBasis>::segN)
        .def("segT", &Geom::Piecewise<Geom::SBasis>::segT)
        .def("offsetDomain", &Geom::Piecewise<Geom::SBasis>::offsetDomain)
        .def("scaleDomain", &Geom::Piecewise<Geom::SBasis>::scaleDomain)
        .def("setDomain", &Geom::Piecewise<Geom::SBasis>::setDomain)
        .def("concat", &Geom::Piecewise<Geom::SBasis>::concat)
        .def("continuousConcat", &Geom::Piecewise<Geom::SBasis>::continuousConcat)
        .def("invariants", &Geom::Piecewise<Geom::SBasis>::invariants)
       
        .def(self + double()) 
        .def(-self)
        .def(self += double())
        .def(self -= double())
        .def(self /= double())
        .def(self *= double())
        .def(self + self)
        .def(self - self)
        .def(self * self)
        .def(self *= self)
        
    ;

    class_<Geom::Piecewise<Geom::D2<Geom::SBasis> > >("PiecewiseD2SBasis")
        .def("__getitem__", getitem_pwsb)
        .def("__call__", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::valueAt)
        .def_readonly("cuts", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::cuts)
        .def_readonly("segs", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::segs)
        .def("valueAt", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::valueAt)
        .def("size", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::size)
        .def("empty", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::empty)
        .def("push", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::push)
        .def("push_cut", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::push_cut)
        .def("push_seg", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::push_seg)

        .def("segN", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::segN)
        .def("segT", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::segT)
        .def("offsetDomain", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::offsetDomain)
        .def("scaleDomain", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::scaleDomain)
        .def("setDomain", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::setDomain)
        .def("concat", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::concat)
        .def("continuousConcat", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::continuousConcat)
        .def("invariants", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::invariants)

        //.def(self + double())
        //.def(-self)
        //.def(self += double())
        //.def(self -= double())
        //.def(self /= double())
        //.def(self *= double())
        //.def(self + self)
        //.def(self - self)
        //.def(self * self)
        //.def(self *= self)

    ;

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
