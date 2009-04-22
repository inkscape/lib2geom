/*
 * Copyright 2009 Nathan Hurst <njh@njhurst.com>
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

#include "py2geom.h"
#include "helpers.h"

#include "../line.h"
#include "../point.h"

using namespace boost::python;

object wrap_intersection(Geom::Line const& a, Geom::Line const& b) {
    Geom::OptCrossing oc = intersection(a, b);
    if(oc) {
        return object(*oc);
    } else
        return object();
}

void wrap_line() {
    //line.h

    def("intersection", wrap_intersection);
    class_<Geom::Line>("Line", init<>())
        .def(init<Geom::Point const&, Geom::Coord>())
        .def(init<Geom::Point const&, Geom::Point const&>())
        .def(init<double, double, double>())
        //.def(self_ns::str(self))
        .def("valueAt", &Geom::Line::valueAt)

        .def("implicit_form_coefficients", &Geom::Line::implicit_form_coefficients)
        .def("isDegenerate", &Geom::Line::isDegenerate)
        .def("pointAt", &Geom::Line::pointAt)
        .def("roots", &Geom::Line::roots)
        .def("nearestPoint", &Geom::Line::nearestPoint)
        .def("reverse", &Geom::Line::reverse)
        //.def("portion", &Geom::Line::portion)
        //.def("segment", &Geom::Line::segment)
        .def("derivative", &Geom::Line::derivative)
        .def("transformed", &Geom::Line::transformed)
        .def("normal", &Geom::Line::normal)
        .def("normalAndDist", &Geom::Line::normalAndDist)
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
