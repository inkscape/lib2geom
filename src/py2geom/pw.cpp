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

#include "2geom/sbasis.h"
#include "2geom/piecewise.h"
#include "2geom/d2.h"
#include "2geom/sbasis-math.h"
#include "2geom/sbasis-geometric.h"

#include "py2geom.h"
#include "helpers.h"

using namespace boost::python;

// helpers for point
tuple pwd2sb_centroid(Geom::Piecewise<Geom::D2<Geom::SBasis> > const &pw)
{
    Geom::Point p;
    double a;
    Geom::centroid(pw, p, a);
    return boost::python::make_tuple(p, a);
}
     

void (Geom::Piecewise<Geom::SBasis>::*push_pwsb)(Geom::SBasis const &, double) = &Geom::Piecewise<Geom::SBasis>::push;
void (Geom::Piecewise<Geom::SBasis>::*push_seg_pwsb)(Geom::SBasis const &) = &Geom::Piecewise<Geom::SBasis>::push_seg;
Geom::Piecewise<Geom::SBasis> (*portion_pwsb)(const Geom::Piecewise<Geom::SBasis> &, double, double) = &Geom::portion;
void (Geom::Piecewise<Geom::D2<Geom::SBasis>>::*push_pwd2sb)(Geom::D2<Geom::SBasis> const &, double) = &Geom::Piecewise<Geom::D2<Geom::SBasis>>::push;
void (Geom::Piecewise<Geom::D2<Geom::SBasis>>::*push_seg_pwd2sb)(Geom::D2<Geom::SBasis> const &) = &Geom::Piecewise<Geom::D2<Geom::SBasis>>::push_seg;
Geom::Piecewise<Geom::D2<Geom::SBasis>> (*portion_pwd2sb)(const Geom::Piecewise<Geom::D2<Geom::SBasis> > &, double, double) = &Geom::portion;
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
Geom::FragmentConcept<Geom::SBasis>::BoundsType (*bounds_local_pwsb)(Geom::Piecewise<Geom::SBasis> const &, const Geom::OptInterval &) = &Geom::bounds_local;

Geom::SBasis getitem_pwsb(Geom::Piecewise<Geom::SBasis> const &p, int index) {
    unsigned D = p.size();
    unsigned i = index;
    if (index < 0)
    {
        i = index = D + index;
    }
    if (index < 0 || i > (D - 1)) {
        PyErr_SetString(PyExc_IndexError, "index out of range");
        boost::python::throw_error_already_set();
    }
    return p[i];
}

Geom::Piecewise<Geom::D2<Geom::SBasis> > (*unitVector_pwd2sb)(Geom::Piecewise<Geom::D2<Geom::SBasis> > const &, double, unsigned int) = &Geom::unitVector;

Geom::Piecewise<Geom::SBasis> (*arcLengthSb_pwd2sb)(Geom::Piecewise<Geom::D2<Geom::SBasis> > const &, double) = &Geom::arcLengthSb;

Geom::Piecewise<Geom::D2<Geom::SBasis> > (*rot90_pwd2sb)(Geom::Piecewise<Geom::D2<Geom::SBasis> > const &) = &Geom::rot90;

void wrap_pw() {
    class_<std::vector<Geom::SBasis> >("SBasisVec")
        .def(vector_indexing_suite<std::vector<Geom::SBasis> >())
    ;
    class_<std::vector<Geom::D2<Geom::SBasis> > >("D2SBasisVec")
        .def(vector_indexing_suite<std::vector<Geom::D2<Geom::SBasis> > >())
    ;

    def("portion", portion_pwsb);
    def("portion", portion_pwd2sb);
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
    
    def("derivative", (Geom::Piecewise<Geom::SBasis> (*)(Geom::Piecewise<Geom::SBasis>  const & ))&Geom::derivative);
    def("integral", (Geom::Piecewise<Geom::SBasis> (*)(Geom::Piecewise<Geom::SBasis>  const & ))&Geom::integral);
    def("derivative", (Geom::Piecewise<Geom::D2<Geom::SBasis> > (*)(Geom::Piecewise<Geom::D2<Geom::SBasis> > const &)) &Geom::derivative);
    def("rot90", rot90_pwd2sb);
    def("unit_vector", unitVector_pwd2sb);
    def("arcLengthSb", arcLengthSb_pwd2sb);

    class_<Geom::Piecewise<Geom::SBasis> >("PiecewiseSBasis", init<>())
        .def(init<double>())
        .def(init<Geom::SBasis>())
        .def("__getitem__", getitem_pwsb)
        .def("__call__", &Geom::Piecewise<Geom::SBasis>::valueAt)
        .def_readonly("cuts", &Geom::Piecewise<Geom::SBasis>::cuts)
        .def_readonly("segs", &Geom::Piecewise<Geom::SBasis>::segs)
        .def("at0", &Geom::Piecewise<Geom::SBasis>::firstValue)
        .def("at1", &Geom::Piecewise<Geom::SBasis>::lastValue)
        .def("valueAt", &Geom::Piecewise<Geom::SBasis>::valueAt)
        .def("size", &Geom::Piecewise<Geom::SBasis>::size)
        .def("empty", &Geom::Piecewise<Geom::SBasis>::empty)
        .def("push", push_pwsb)
        .def("push_cut", &Geom::Piecewise<Geom::SBasis>::push_cut)
        .def("push_seg", push_seg_pwsb)

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
        .def(self * double())
        .def(self *= double())
        .def(self + self)
        .def(self - self)
        .def(self * self)
        .def(self *= self)
        
    ;

    class_<Geom::Piecewise<Geom::D2<Geom::SBasis> > >("PiecewiseD2SBasis", init<>())
        .def(init<Geom::D2<Geom::SBasis> >())
        .def("__getitem__", getitem_pwsb)
        .def("__call__", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::valueAt)
        .def_readonly("cuts", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::cuts)
        .def_readonly("segs", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::segs)
        .def("valueAt", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::valueAt)
        .def("size", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::size)
        .def("empty", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::empty)
        .def("push", push_pwd2sb)
        .def("push_cut", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::push_cut)
        .def("push_seg", push_seg_pwd2sb)

        .def("segN", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::segN)
        .def("segT", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::segT)
        .def("offsetDomain", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::offsetDomain)
        .def("scaleDomain", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::scaleDomain)
        .def("setDomain", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::setDomain)
        .def("concat", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::concat)
        .def("continuousConcat", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::continuousConcat)
        .def("invariants", &Geom::Piecewise<Geom::D2<Geom::SBasis> >::invariants)

        //.def(self + double())
        .def(-self)
        //.def(self += double())
        //.def(self -= double())
        //.def(self /= double())
        .def(self * double())
        .def(Geom::Piecewise<Geom::SBasis>() * self)
        .def(self *= double())
        .def(self + self)
        .def(self - self)
        //.def(self * self)
        //.def(self *= self)

    ;
    def("centroid", pwd2sb_centroid);
    def("make_cuts_independent", Geom::make_cuts_independent);

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
