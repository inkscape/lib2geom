/*
 * Python bindings for lib2geom
 *
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
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include "../path.h"
#include "../pathvector.h"
#include "../transforms.h"
#include "helpers.h"

#include "../region.h"
#include "../shape.h"

using namespace boost::python;

void wrap_shape()
{
    class_<Geom::Region>("Region")
        .def("size", &Geom::Region::size)
        .def("isFill", &Geom::Region::isFill)
        .def("asFill", &Geom::Region::asFill)
        .def("asHole", &Geom::Region::asHole)
        .def("boundsFast", &Geom::Region::boundsFast)
        //.def("contains", &Geom::Region::contains)
        .def("includes", &Geom::Region::includes)
        .def("inverse", &Geom::Region::inverse)
        .def(self * Geom::Matrix())
        .def("invariants", &Geom::Region::invariants)
    ;
    class_<Geom::Regions>("Regions")
        //.def(vector_indexing_suite<Geom::Regions>())
    ;
    def("regions_from_paths", Geom::regions_from_paths);
    def("paths_from_regions", Geom::paths_from_regions);
    def("sanitize_path", Geom::sanitize_path);
    class_<Geom::Shape>("Shape")
        .def("getContent", &Geom::Shape::getContent)
        .def("isFill", &Geom::Shape::isFill)
        .def("size", &Geom::Shape::size)
        .def("inverse", &Geom::Shape::inverse)
        .def(self * Geom::Matrix())
        .def("contains", &Geom::Shape::contains)
        .def("inside_invariants", &Geom::Shape::inside_invariants)
        .def("region_invariants", &Geom::Shape::region_invariants)
        .def("cross_invariants", &Geom::Shape::cross_invariants)
        .def("invariants", &Geom::Shape::invariants)
    ;
    def("sanitize", Geom::sanitize);
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(substatement-open . 0))
  indent-tabs-mode:nil
  c-brace-offset:0
  fill-column:99
  End:
  vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/
