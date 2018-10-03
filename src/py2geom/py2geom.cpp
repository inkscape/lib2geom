/*
 * Python bindings for lib2geom
 *
 * Copyright 2006, 2007 Aaron Spike <aaron@ekips.org>
 * Copyright 2007 Alex Mac <ajm@cs.nott.ac.uk>
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

#include <2geom/geom.h>

#include "py2geom.h"

using namespace boost::python;

BOOST_PYTHON_MODULE(_py2geom)
{
    
    /*enum_<IntersectorKind>("IntersectorKind")
        .value("intersects", intersects)
        .value("parallel", parallel)
        .value("coincident", coincident)
        .value("no_intersection", no_intersection)
    ;
    def("segment_intersect", segment_intersect);*/
    
    wrap_point();
    wrap_etc();
    wrap_interval();
    wrap_transforms();
    wrap_rect();
    wrap_circle();
    wrap_ellipse();
    wrap_sbasis();
    wrap_bezier();
    wrap_linear();
    wrap_line();
    wrap_conic();
    wrap_pw();
    wrap_d2();
    wrap_parser();
    wrap_path();
    wrap_ray();
    // wrap_shape();
    wrap_crossing();
    // wrap_convex_cover();

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
