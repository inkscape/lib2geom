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

#include "2geom/point.h"
#include "2geom/exception.h"
#include "2geom/convex-cover.h"



using namespace boost::python;

PointVec ch_boundary(Geom::ConvexHull const &ch) {
    return ch.boundary;
}

int furthest_index(Geom::ConvexHull const &ch, Geom::Point const &p) {
    return (int)(ch.furthest(p) - &ch.boundary[0]);
}

void wrap_convex_cover() {
    class_<Geom::ConvexHull>("ConvexHull", init<>())
        .def(init<PointVec>())
      
        .def("merge", &Geom::ConvexHull::merge)
        .def("contains_point", &Geom::ConvexHull::contains_point)
        .def("strict_contains_point", &Geom::ConvexHull::strict_contains_point)

        .add_property("boundary", &ch_boundary)
        .add_property("is_clockwise", &Geom::ConvexHull::is_clockwise)
        .add_property("top_point_first", &Geom::ConvexHull::top_point_first)
        .add_property("meets_invariants", &Geom::ConvexHull::meets_invariants)
        .add_property("empty", &Geom::ConvexHull::empty)

        .add_property("singular", &Geom::ConvexHull::singular)

        .add_property("linear", &Geom::ConvexHull::linear)
        .add_property("is_degenerate", &Geom::ConvexHull::is_degenerate)

        .def("centroid_and_area", &Geom::ConvexHull::centroid_and_area)
        .def("area", &Geom::ConvexHull::area)
        .def("furthest", &furthest_index)

        .def("is_left", &Geom::ConvexHull::is_left)
        .def("is_strict_left", &Geom::ConvexHull::is_strict_left)
        .def("find_left", &Geom::ConvexHull::find_left)
        .def("find_strict_left", &Geom::ConvexHull::find_strict_left)
        .def("narrowest_diameter", &Geom::ConvexHull::narrowest_diameter)
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
