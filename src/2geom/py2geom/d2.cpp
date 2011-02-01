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
#include <2geom/point.h>
#include <2geom/sbasis.h>
#include <2geom/d2.h>
#include <2geom/piecewise.h>

using namespace boost::python;

void wrap_d2() {
    class_<Geom::D2<Geom::SBasis> >("D2SBasis", init<>())
        .def(init<Geom::SBasis,Geom::SBasis>())
        .def("__getitem__", python_getitem<Geom::D2<Geom::SBasis>,Geom::SBasis,2>)

        .def("isZero", &Geom::D2<Geom::SBasis>::isZero)
        .def("isFinite", &Geom::D2<Geom::SBasis>::isFinite)
        .def("at0", &Geom::D2<Geom::SBasis>::at0)
        .def("at1", &Geom::D2<Geom::SBasis>::at1)
        .def("pointAt", &Geom::D2<Geom::SBasis>::valueAt)
	.def("valueAndDerivatives", &Geom::D2<Geom::SBasis>::valueAndDerivatives)
        .def("toSBasis", &Geom::D2<Geom::SBasis>::toSBasis)

        .def(-self)
        .def(self + self)
        .def(self - self)
        .def(self += self)
        .def(self -= self)
        .def(self + Geom::Point())
        .def(self - Geom::Point())
        .def(self += Geom::Point())
        .def(self -= Geom::Point())
        .def(self * Geom::Point())
        .def(self / Geom::Point())
        .def(self *= Geom::Point())
        .def(self /= Geom::Point())
        .def(self * float())
        .def(self / float())
        .def(self *= float())
        .def(self /= float())
    ;
    def("reverse", ((Geom::D2<Geom::SBasis> (*)(Geom::D2<Geom::SBasis> const &b))&Geom::reverse));
    def("portion", ((Geom::D2<Geom::SBasis> (*)(Geom::D2<Geom::SBasis> const &a, Geom::Coord f, Geom::Coord t))&Geom::portion));
    //TODO: dot, rot90, cross, compose, composeEach, eval ops, derivative, integral, L2, portion, multiply ops, 
    
    class_<Geom::D2<Geom::Piecewise<Geom::SBasis> > >("D2PiecewiseSBasis")
        .def("__getitem__", python_getitem<Geom::D2<Geom::Piecewise<Geom::SBasis> >,Geom::Piecewise<Geom::SBasis>,2>)

        //.def("isZero", &Geom::D2<Geom::Piecewise<Geom::SBasis> >::isZero)
        //.def("isFinite", &Geom::D2<Geom::Piecewise<Geom::SBasis> >::isFinite)
        //.def("at0", &Geom::D2<Geom::Piecewise<Geom::SBasis> >::at0)
        //.def("at1", &Geom::D2<Geom::Piecewise<Geom::SBasis> >::at1)
        //.def("pointAt", &Geom::D2<Geom::Piecewise<Geom::SBasis> >::valueAt)
        //.def("valueAndDerivatives", &Geom::D2<Geom::Piecewise<Geom::SBasis> >::valueAndDerivatives)
        //.def("toSBasis", &Geom::D2<Geom::Piecewise<Geom::SBasis> >::toSBasis)

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
