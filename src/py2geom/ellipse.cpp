/*
 * Copyright 2009 Ricardo Lafuente <r@sollec.org>
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

#include "2geom/point.h"
#include "2geom/ellipse.h"
#include "2geom/circle.h"
#include "2geom/exception.h"
#include "2geom/d2.h"


void  (Geom::Ellipse::*ellipse_set1)(Geom::Point const &, Geom::Point const &, double) = &Geom::Ellipse::set;
void  (Geom::Ellipse::*ellipse_set2)(double, double, double, double, double) = &Geom::Ellipse::set;
std::vector<Geom::Coord> (Geom::Ellipse::*ellipse_coefficients)() const = &Geom::Ellipse::coefficients;

// i can't get these to work
//Geom::Point  (Geom::Ellipse::*center_point)() = (Geom::Point (*)() const)&Geom::Ellipse::center;
// Geom::Coord  (Geom::Ellipse::*center_coord)(Geom::Dim2 const& d) = &Geom::Ellipse::center;

using namespace boost::python;

void wrap_ellipse() {
    class_<Geom::Ellipse>("Ellipse", init<double, double, double, double, double>())
        .def(init<double, double, double, double, double, double>())
        // needs to be mapped to PointVec, but i can't figure out how
        .def(init<Geom::Circle>())

        .def("set", ellipse_set1)
        .def("set", ellipse_set2)
        .def("setCoefficients", &Geom::Ellipse::setCoefficients)
        .def("fit", &Geom::Ellipse::fit)
        
        .def("center", (Geom::Point (Geom::Ellipse::*)() const) &Geom::Ellipse::center)
        // .def("center", center_coord)
        
        .def("ray", &Geom::Ellipse::ray)
        .def("rotationAngle", &Geom::Ellipse::rotationAngle)
        .def("coefficients", ellipse_coefficients)
        .def(self * Geom::Affine())
        .def(self *= Geom::Affine())
        // requires EllipticalArc
        //.def("arc", &Geom::Ellipse::arc)
        
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
