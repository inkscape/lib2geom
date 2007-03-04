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

Geom::pw_sb (*portion_pwsb)(const Geom::pw_sb &, double, double) = &Geom::portion;
std::vector<double> (*roots_pwsb)(const Geom::pw_sb &) = &Geom::roots;
Geom::pw_sb (*multiply_pwsb)(Geom::pw_sb const &, Geom::pw_sb const &) = &Geom::multiply;
Geom::pw_sb (*divide_pwsb)(Geom::pw_sb const &, Geom::pw_sb const &, int) = &Geom::divide;
Geom::pw_sb (*compose_pwsb_sb)(Geom::pw_sb const &, Geom::SBasis const &) = &Geom::compose;
Geom::pw_sb (*compose_pwsb)(Geom::pw_sb const &, Geom::pw_sb const &) = &Geom::compose;

Geom::SBasis getitem_pwsb(Geom::pw_sb const &p, int const index) {
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

double call_pwsb(Geom::pw_sb const &p, double t) {
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
    
    class_<Geom::pw_sb>("pw_sb")
        .def("__getitem__", getitem_pwsb)
        .def("__call__", call_pwsb)
        .def_readonly("cuts", &Geom::pw_sb::cuts)
        .def_readonly("segs", &Geom::pw_sb::segs)
        .def("size", &Geom::pw_sb::size)
        .def("empty", &Geom::pw_sb::empty)
        .def("push", &Geom::pw_sb::push)
        .def("push_cut", &Geom::pw_sb::push_cut)
        .def("push_seg", &Geom::pw_sb::push_seg)

        .def("segn", &Geom::pw_sb::segn)
        .def("segt", &Geom::pw_sb::segt)
        .def("offsetDomain", &Geom::pw_sb::offsetDomain)
        .def("scaleDomain", &Geom::pw_sb::scaleDomain)
        .def("setDomain", &Geom::pw_sb::setDomain)
        .def("concat", &Geom::pw_sb::concat)
        .def("continuousConcat", &Geom::pw_sb::continuousConcat)
        .def("invariants", &Geom::pw_sb::invariants)
       
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
