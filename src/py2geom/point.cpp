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

#include "2geom/point.h"

using namespace boost::python;


// helpers for point
tuple point_to_tuple(Geom::Point const& p)
{
    return make_tuple(p[0], p[1]);
}

Geom::Point tuple_to_point(boost::python::tuple const& t)
{
    return Geom::Point(extract<double>(t[0]), extract<double>(t[1]));
}

str point_repr(Geom::Point const& p)
{
    return str("(" + str(p[0]) + ", " + str(p[1]) + ")");
}

//Specifications of overloads
Geom::Coord (*L2_point)   (Geom::Point const &) = &Geom::L2;
Geom::Point (*rot90_point)(Geom::Point const &) = &Geom::rot90;
Geom::Coord (*dot_point)  (Geom::Point const &, Geom::Point const &) = &Geom::dot;
Geom::Coord (*cross_point)(Geom::Point const &, Geom::Point const &) = &Geom::cross;
Geom::Point (*lerp_point)(Geom::Coord, Geom::Point const &, Geom::Point const &) = &Geom::lerp;

bool near_point1(Geom::Point const &a, Geom::Point const &b) { return are_near(a,b); }
bool near_point2(Geom::Point const &a, Geom::Point const &b, double eps) { return are_near(a,b,eps); }

void wrap_point() {
    def("point_to_tuple", point_to_tuple);
    def("tuple_to_point", tuple_to_point);

    def("L1", Geom::L1);
    def("L2", L2_point);
    def("L2sq", Geom::L2sq);
    def("LInfty", Geom::LInfty);

    def("unit_vector", Geom::unit_vector);
    def("is_zero", Geom::is_zero);
    def("is_unit_vector", Geom::is_unit_vector);

    def("dot", dot_point);
    def("cross", cross_point);
    def("distance", Geom::distance);
    def("distanceSq", Geom::distanceSq);
    def("lerp", lerp_point);

    def("atan2", Geom::atan2);
    def("angle_between", Geom::angle_between);

    def("near", near_point1);
    def("near", near_point2);

    def("rot90", rot90_point);
    def("abs", (Geom::Point (*)(Geom::Point const&))&Geom::abs);

    class_<Geom::Point>("Point", init<double, double>())
        .def(init<>())
        
        .def("__str__", point_repr)
        .def("__repr__", point_repr)
        .def("__getitem__", python_getitem<Geom::Point,double,2>)
        .def("tuple", point_to_tuple)
    
        .def("from_tuple", tuple_to_point)
        .staticmethod("from_tuple")
        
        //point.h
        //.def("polar", &Geom::Point::polar)
        //.staticmethod("polar")

        .def("ccw", &Geom::Point::ccw)
        .def("cw", &Geom::Point::cw)
        .def("round", &Geom::Point::round)
        .def("normalize", &Geom::Point::normalize)

        .def("length", &Geom::Point::length)

        .def(self + self)
        .def(self - self)
        .def(self += self)
        .def(self -= self)

        .def(-self)
        .def(self * float()).def(float() * self)
        .def(self / float())
        .def(self *= float())

        .def(self == self)
        .def(self != self)

        .def(self <= self)
    ;
    implicitly_convertible<Geom::Point,tuple>();
// TODO: explain why this gives a compile time error
//    implicitly_convertible<tuple,Geom::Point>();

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
