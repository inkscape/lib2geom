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

#include "py2geom.h"
#include "helpers.h"

#include "2geom/linear.h"
#include "2geom/point.h"
#include "2geom/sbasis.h"

using namespace boost::python;

// helpers for bezord
tuple bezord_to_tuple(Geom::Linear const& b)
{
    return make_tuple(b[0], b[1]);
}

Geom::Linear tuple_to_bezord(boost::python::tuple const& t)
{
    return Geom::Linear(extract<double>(t[0]), extract<double>(t[1]));
}

str bezord_repr(Geom::Linear const& b)
{
    return str("<" + str(b[0]) + ", " + str(b[1]) + ">");
}

void wrap_linear() {
    def("lerp", (double (*)(double, double, double))&Geom::lerp);
    def("reverse", (Geom::Linear (*)(Geom::Linear const &))&Geom::reverse);
    def("bounds_fast", (Geom::OptInterval (*)(Geom::Linear const &))&Geom::bounds_fast);
    def("bounds_exact", (Geom::OptInterval (*)(Geom::Linear const &))&Geom::bounds_exact);
    def("bounds_local", (Geom::OptInterval (*)(Geom::Linear const &))&Geom::bounds_local);

    class_<Geom::Linear>("Linear", init<double, double>())
        .def("__str__", bezord_repr)
        .def("__repr__", bezord_repr)
        .def("__getitem__", python_getitem<Geom::Linear,double,2>)
        .def("tuple", bezord_to_tuple)

        .def("from_tuple", tuple_to_bezord)
        .staticmethod("from_tuple")

        .def("isZero", &Geom::Linear::isZero)
        .def("isFinite", &Geom::Linear::isFinite)
        .def("at0", (double (Geom::Linear::*)() const) &Geom::Linear::at0)
        .def("at1", (double (Geom::Linear::*)() const) &Geom::Linear::at1)
        .def("valueAt", &Geom::Linear::valueAt)
        .def("toSBasis", &Geom::Linear::toSBasis)

        .def(-self)
        .def(self + self)
        .def(self - self)
        .def(self += self)
        .def(self -= self)
        .def(self + float())
        .def(self - float())
        .def(self += float())
        .def(self -= float())
        .def(self == self)
        .def(self != self)
        .def(self * float())
        .def(self / float())
        .def(self *= float())
        .def(self /= float())
    ;
    def("reverse", ((Geom::Linear (*)(Geom::Linear const &b))&Geom::reverse));
    //TODO: reinstate
    //implicitly_convertible<Geom::Linear,tuple>();
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
