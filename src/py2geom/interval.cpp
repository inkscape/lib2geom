/*
 * Copyright 2008 Aaron Spike <aaron@ekips.org>
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

#include "2geom/interval.h"

using namespace boost::python;


// helpers for interval
static tuple interval_to_tuple(Geom::Interval const& p)
{
    return make_tuple(p.min(), p.max());
}

static Geom::Interval tuple_to_interval(boost::python::tuple const& t)
{
    return Geom::Interval(extract<double>(t[0]), extract<double>(t[1]));
}

static str interval_repr(Geom::Interval const& p)
{
    return str("(" + str(p.min()) + ", " + str(p.max()) + ")");
}

static Geom::Interval from_optinterval(Geom::OptInterval const & ivl)
{
    return *ivl;
}


static bool wrap_contains_coord(Geom::Interval const &x, Geom::Coord val) {
    return x.contains(val);
}

static bool wrap_contains_ivl(Geom::Interval const &x, Geom::Interval val) {
    return x.contains(val);
}

static bool wrap_interiorContains_coord(Geom::Interval const &x, Geom::Coord val) {
    return x.interiorContains(val);
}

static bool wrap_interiorContains_ivl(Geom::Interval const &x, Geom::Interval val) {
    return x.interiorContains(val);
}

void wrap_interval() {
    def("interval_to_tuple", interval_to_tuple);
    def("tuple_to_interval", tuple_to_interval);

    //def("unify", Geom::unify(Geom::Interval const &, Geom::Interval const &));
    //def("intersect", Geom::intersect(Geom::Interval const &, Geom::Interval const &));

    //TODO: add overloaded constructors
    class_<Geom::Interval>("Interval", init<double, double>())
        .def("__str__", interval_repr)
        .def("__repr__", interval_repr)
        .def("tuple", interval_to_tuple)
    
        .def("from_tuple", tuple_to_interval)
        .staticmethod("from_tuple")
        
        .def("min", &Geom::Interval::min)
        .def("max", &Geom::Interval::max)
        .def("middle", &Geom::Interval::middle)
        .def("extent", &Geom::Interval::extent)
        .def("isSingular", &Geom::Interval::isSingular)
        //TODO: fix for overloading
        .def("contains", wrap_contains_coord)
        .def("contains", wrap_contains_ivl)
        .def("interiorContains", wrap_interiorContains_coord)
        .def("interiorContains", wrap_interiorContains_ivl)
        .def("intersects", &Geom::Interval::intersects)

        .def("setMin", &Geom::Interval::setMin)
        .def("setMax", &Geom::Interval::setMax)
        .def("expandTo", &Geom::Interval::expandTo)
        .def("from_array", &Geom::Interval::from_array)
        .def("expandBy", &Geom::Interval::expandBy)
        .def("unionWith", &Geom::Interval::unionWith)

        .def(self == self)
        .def(self != self)

        .def(self + float())
        .def(self - float())
        .def(self += float())
        .def(self -= float())

        .def(-self)

        .def(self * float())
        .def(self / float())
        .def(self *= float())
        .def(self /= float())

        .def(self + self)
        .def(self - self)
        .def(self += self)
        .def(self -= self)
        .def(self * self)
        .def(self *= self)
    ;
    class_<Geom::OptInterval>("OptInterval", init<double, double>())
        .def(init<Geom::Interval>())
        .def("unionWith", &Geom::OptInterval::unionWith)
        .def("empty", &Geom::OptInterval::empty)
        .def("toInterval", from_optinterval)

        .def(self == self)
        .def(self != self)
    ;
    implicitly_convertible<Geom::Interval,tuple>();
// TODO: is this possible?
//    implicitly_convertible<tuple,Geom::Interval>();

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
