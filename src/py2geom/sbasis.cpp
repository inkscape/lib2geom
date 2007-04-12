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

#include "sbasis.h"
#include "helpers.h"

#include "s-basis.h"
#include "../point.h"

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

Geom::SBasis (*truncate_sbasis)(Geom::SBasis const &, unsigned) = &Geom::truncate;
Geom::SBasis (*multiply_sbasis)(Geom::SBasis const &, Geom::SBasis const &) = &Geom::multiply;
Geom::SBasis (*integral_sbasis)(Geom::SBasis const &) = &Geom::integral;
Geom::SBasis (*derivative_sbasis)(Geom::SBasis const &) = &Geom::derivative;

void wrap_sbasis() {
    //s-basis.h
    class_<Geom::Linear>("Linear", init<double, double>())
        .def("__str__", bezord_repr)
        .def("__repr__", bezord_repr)
        .def("__getitem__", python_getitem<Geom::Linear,double,2>)
        .def("tuple", bezord_to_tuple)

        .def("from_tuple", tuple_to_bezord)
        .staticmethod("from_tuple")

        .def("isZero", &Geom::Linear::isZero)
        .def("isFinite", &Geom::Linear::isFinite)
        .def("at0", &Geom::Linear::at0)
        .def("at1", &Geom::Linear::at1)
        .def("pointAt", &Geom::Linear::pointAt)
        .def("toSBasis", &Geom::Linear::toSBasis)
        .def("reverse", &Geom::Linear::reverse)

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
        .def(float() * self)
        .def(self *= float())
    ;
    implicitly_convertible<Geom::Linear,tuple>();
// TODO: explain why this gives a compile time error
//    implicitly_convertible<tuple,Geom::Linear>();

    // needed for roots
    class_<std::vector<double> >("DoubleVec")
        .def(vector_indexing_suite<std::vector<double> >())
    ;
    class_<std::vector<Geom::Point> >("PointVec")
        .def(vector_indexing_suite<std::vector<Geom::Point> >())
    ;
    // sbasis is a subclass of
    class_<std::vector<Geom::Linear> >("LinearVec")
        .def(vector_indexing_suite<std::vector<Geom::Linear> >())
    ;

    def("shift", (Geom::SBasis (*)(Geom::SBasis const &a, int sh))&Geom::shift);
    def("truncate", truncate_sbasis);
    def("multiply", multiply_sbasis);
    def("compose", (Geom::SBasis (*) (Geom::SBasis const &, Geom::SBasis const &))&Geom::compose);
    def("integral", integral_sbasis);
    def("derivative", derivative_sbasis);
    def("sqrt", &Geom::sqrt);
    def("reciprocal", &Geom::reciprocal);
    def("divide", &Geom::divide);
    def("inverse", &Geom::inverse);
    def("sin", &Geom::sin);
    def("cos", &Geom::cos);
    def("bounds", &Geom::bounds);
    def("roots", &Geom::roots);

    class_<Geom::SBasis, bases<std::vector<Geom::Linear> > >("SBasis")
        .def(self_ns::str(self))
        //TODO: add important vector funcs

        .def("isZero", &Geom::SBasis::isZero)
        .def("isFinite", &Geom::SBasis::isFinite)
        .def("at0", &Geom::SBasis::at0)
        .def("at1", &Geom::SBasis::at1)
        .def("pointAt", &Geom::SBasis::pointAt)
        .def("toSBasis", &Geom::SBasis::toSBasis)
        .def("reverse", &Geom::SBasis::reverse)

        .def("normalize", &Geom::SBasis::normalize)
        .def("tailError", &Geom::SBasis::tailError)
        .def("truncate", &Geom::SBasis::truncate)
        .def(self + self)
        .def(self - self)
        .def(self += self)
        .def(self -= self)
        .def(Geom::Linear() + self)
        .def(Geom::Linear() - self)
        .def(self += Geom::Linear())
        .def(self -= Geom::Linear())
        .def(float() + self)
        .def(self += float())
        .def(self -= float())
        
        .def(self * self)
        .def(float() * self)
        .def(self *= self)
        .def(self *= float())
        .def(self /= float())
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
