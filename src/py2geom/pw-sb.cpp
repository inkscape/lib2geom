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

#include "../s-basis.h"
#include "../pw-sb.h"

#include "pw-sb.h"
#include "helpers.h"

using namespace boost::python;

Geom::Piecewise<SBasis> (*portion_pwsb)(const Geom::Piecewise<SBasis> &, double, double) = &Geom::portion;
std::vector<double> (*roots_pwsb)(const Geom::Piecewise<SBasis> &) = &Geom::roots;
Geom::Piecewise<SBasis> (*multiply_pwsb)(Geom::Piecewise<SBasis> const &, Geom::Piecewise<SBasis> const &) = &Geom::multiply;
Geom::Piecewise<SBasis> (*divide_pwsb)(Geom::Piecewise<SBasis> const &, Geom::Piecewise<SBasis> const &, int) = &Geom::divide;
Geom::Piecewise<SBasis> (*compose_pwsb_sb)(Geom::Piecewise<SBasis> const &, Geom::SBasis const &) = &Geom::compose;
Geom::Piecewise<SBasis> (*compose_pwsb)(Geom::Piecewise<SBasis> const &, Geom::Piecewise<SBasis> const &) = &Geom::compose;

Geom::SBasis getitem_pwsb(Geom::Piecewise<SBasis> const &p, int const index) {
    unsigned D = p.size();
    int i = index;
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

double call_pwsb(Geom::Piecewise<SBasis> const &p, double t) {
    return p(t);
}

void wrap_pwsb() {
    class_<std::vector<Geom::SBasis> >("SBasisVec")
        .def(vector_indexing_suite<std::vector<Geom::SBasis> >())
    ;

    def("portion", portion_pwsb);
    def("partition", &Geom::partition);
    def("roots", roots_pwsb);
    def("multiply", multiply_pwsb);
    def("divide", divide_pwsb);
    def("compose", compose_pwsb_sb);
    def("compose", compose_pwsb);
    
    class_<Geom::Piecewise<SBasis> >("Piecewise<SBasis>")
        .def("__getitem__", getitem_pwsb)
        .def("__call__", call_pwsb)
        .def_readonly("cuts", &Geom::Piecewise<SBasis>::cuts)
        .def_readonly("segs", &Geom::Piecewise<SBasis>::segs)
        .def("size", &Geom::Piecewise<SBasis>::size)
        .def("empty", &Geom::Piecewise<SBasis>::empty)
        .def("push", &Geom::Piecewise<SBasis>::push)
        .def("push_cut", &Geom::Piecewise<SBasis>::push_cut)
        .def("push_seg", &Geom::Piecewise<SBasis>::push_seg)

        .def("segn", &Geom::Piecewise<SBasis>::segn)
        .def("segt", &Geom::Piecewise<SBasis>::segt)
        .def("offsetDomain", &Geom::Piecewise<SBasis>::offsetDomain)
        .def("scaleDomain", &Geom::Piecewise<SBasis>::scaleDomain)
        .def("setDomain", &Geom::Piecewise<SBasis>::setDomain)
        .def("concat", &Geom::Piecewise<SBasis>::concat)
        .def("continuousConcat", &Geom::Piecewise<SBasis>::continuousConcat)
        .def("invariants", &Geom::Piecewise<SBasis>::invariants)
       
        .def(+self)
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
