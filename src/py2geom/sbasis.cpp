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
tuple bezord_to_tuple(Geom::BezOrd const& b)
{
    return make_tuple(b[0], b[1]);
}

Geom::BezOrd tuple_to_bezord(boost::python::tuple const& t)
{
    return Geom::BezOrd(extract<double>(t[0]), extract<double>(t[1]));
}

str bezord_repr(Geom::BezOrd const& b)
{
    return str("<" + str(b[0]) + ", " + str(b[1]) + ">");
}

Geom::SBasis (*truncate_sbasis)(Geom::SBasis const &, unsigned) = &Geom::truncate;
Geom::SBasis (*multiply_sbasis)(Geom::SBasis const &, Geom::SBasis const &) = &Geom::multiply;
Geom::SBasis (*integral_sbasis)(Geom::SBasis const &) = &Geom::integral;
Geom::SBasis (*derivative_sbasis)(Geom::SBasis const &) = &Geom::derivative;

void wrap_sbasis() {
    //s-basis.h
    class_<Geom::BezOrd>("BezOrd", init<double, double>())
        .def("__str__", bezord_repr)
        .def("__repr__", bezord_repr)
        .def("__getitem__", python_getitem<Geom::BezOrd,double,2>)
        .def("tuple", bezord_to_tuple)

        .def("from_tuple", tuple_to_bezord)
        .staticmethod("from_tuple")

        .def("point_at", &Geom::BezOrd::point_at)
        .def("apply", &Geom::BezOrd::apply)
        .def("zero", &Geom::BezOrd::zero)
        .def("is_finite", &Geom::BezOrd::is_finite)

        .def(-self)
        .def(self + self)
        .def(self - self)
        .def(self += self)
        .def(self -= self)
        .def(self == self)
        .def(self != self)
        .def(self * self)
        .def("reverse", ((Geom::BezOrd (*)(Geom::BezOrd const &b))&Geom::reverse))
    ;
    implicitly_convertible<Geom::BezOrd,tuple>();
// TODO: explain why this gives a compile time error
//    implicitly_convertible<tuple,Geom::BezOrd>();

    // needed for roots
    class_<std::vector<double> >("DoubleVec")
        .def(vector_indexing_suite<std::vector<double> >())
    ;
    class_<std::vector<Geom::Point> >("PointVec")
        .def(vector_indexing_suite<std::vector<Geom::Point> >())
    ;
    // sbasis is a subclass of
    class_<std::vector<Geom::BezOrd> >("BezOrdVec")
        .def(vector_indexing_suite<std::vector<Geom::BezOrd> >())
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
    def("reverse", (Geom::SBasis (*)(Geom::SBasis const &))&Geom::reverse);
    def("bounds", &Geom::bounds);
    def("roots", &Geom::roots);

    class_<Geom::SBasis, bases<std::vector<Geom::BezOrd> > >("SBasis")
        .def(self_ns::str(self))
        .def(self + self)
        .def(self - self)
        .def("clear", &Geom::SBasis::clear)
        .def("normalize", &Geom::SBasis::normalize)
        .def("tail_eror", &Geom::SBasis::tail_error)
        .def("truncate", &Geom::SBasis::truncate)
        .def("is_finite", &Geom::SBasis::is_finite)
        .def(Geom::BezOrd() - self)
        .def(self += self)
        .def(self -= self)
        .def(self += Geom::BezOrd())
        .def(self -= Geom::BezOrd())
        .def(self += float())
        .def(self -= float())
        .def(Geom::BezOrd() + self)
        .def(float() + self)
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
