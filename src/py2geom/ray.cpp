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
#include "2geom/ray.h"
// #include "2geom/bezier_curve.h"
#include "2geom/exception.h"


using namespace boost::python;

bool (*are_near_ray)(Geom::Point const& _point, Geom::Ray const& _ray, double eps) = &Geom::are_near;
double (*angle_between_ray)(Geom::Ray const& r1, Geom::Ray const& r2, bool cw) = &Geom::angle_between;


double angle_between_ray_def(Geom::Ray const& r1, Geom::Ray const& r2) {
    return Geom::angle_between(r1, r2);
}
double (*distance_ray)(Geom::Point const& _point, Geom::Ray const& _ray) = &Geom::distance;

// why don't these compile?
//Geom::Point (*get_ray_origin)(Geom::Ray const) = &Geom::Ray::origin;
//void (*set_ray_origin)(Geom::Ray const, Geom::Point const& _point) = &Geom::Ray::origin;

void wrap_ray() {
    def("distance", distance_ray);
    def("are_near", are_near_ray);
    def("are_same", Geom::are_same);
    def("angle_between", angle_between_ray);
    def("angle_between", angle_between_ray_def);
    def("make_angle_bisector_ray", Geom::make_angle_bisector_ray);

    class_<Geom::Ray>("Ray", init<Geom::Point, Geom::Coord>())
        .def(init<Geom::Point,Geom::Point>())
        .def(init<>())
            
        // TODO: overloaded
        //.add_property("origin", get_ray_origin, set_ray_origin) 
        // .add_property("versor", &Geom::Ray::versor, &Geom::Ray::versor)
        // .add_property("angle", &Geom::Ray::angle, &Geom::Ray::angle)

        .def("isDegenerate", &Geom::Ray::isDegenerate)
        .def("nearestTime", &Geom::Ray::nearestTime) 
        .def("setBy2Points", &Geom::Ray::setPoints)
        .def("valueAt", &Geom::Ray::valueAt)
        .def("pointAt", &Geom::Ray::pointAt)
        .def("nearestTime", &Geom::Ray::nearestTime)
        .def("reverse", &Geom::Ray::reverse) 
        .def("roots", &Geom::Ray::roots) 
        .def("transformed", &Geom::Ray::transformed) 
        // requires Curve
        // .def("portion", &Geom::Ray::portion) 
        .def("segment", &Geom::Ray::segment) 
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
