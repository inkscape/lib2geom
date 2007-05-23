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
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include "py2geom.h"
#include "helpers.h"
#include "../d2.h"

using namespace boost::python;

typedef Geom::D2<Geom::SBasis> SBasis2D;
typedef Geom::D2<Geom::Piecewise<Geom::SBasis> > d2pwsb;


void wrap_d2() {
    class_<SBasis2D>("SBasis2D")
        .def("__getitem__", python_getitem<SBasis2D,Geom::SBasis,2>)

        .def("isZero", &Geom::Linear::isZero)
        .def("isFinite", &Geom::Linear::isFinite)
        .def("at0", &Geom::Linear::at0)
        .def("at1", &Geom::Linear::at1)
        .def("pointAt", &Geom::Linear::pointAt)
        .def("toSBasis", &Geom::Linear::toSBasis)

        .def(-self)
        .def(self + self)
        .def(self - self)
        .def(self += self)
        .def(self -= self)
        .def(self + Geom::Point)
        .def(self - Geom::Point)
        .def(self += Geom::Point)
        .def(self -= Geom::Point)
        .def(self * Geom::Point)
        .def(self / Geom::Point)
        .def(self *= Geom::Point)
        .def(self /= Geom::Point)
        .def(self * float())
        .def(self / float())
        .def(self *= float())
        .def(self /= float())
    ;
    def("reverse", ((SBasis2D (*)(SBasis2D const &b))&Geom::reverse));
    //TODO: dot, rot90, cross, compose, composeEach, eval ops, derivative, integral, L2, portion, multiply ops, 
    
    class_<MultidimSBasis3>("MultidimSBasis3")
        .def("__getitem__", python_getitem<MultidimSBasis2,Geom::SBasis,3>)
        .def("size", &MultidimSBasis3::size)
        .def("tail_error", &MultidimSBasis3::tail_error)
        .def("is_finite", &MultidimSBasis3::is_finite)
        .def(self + self)
        .def(self - self)
        .def(double() * self)
        .def(self += self)
        .def(self *= double())
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
