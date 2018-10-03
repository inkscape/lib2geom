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

#include "2geom/affine.h"
#include "2geom/d2.h"
#include "2geom/interval.h"

using namespace boost::python;

static bool wrap_contains_coord(Geom::Rect const &x, Geom::Point val) {
    return x.contains(val);
}

static bool wrap_contains_ivl(Geom::Rect const &x, Geom::Rect val) {
    return x.contains(val);
}

static bool wrap_interiorContains_coord(Geom::Rect const &x, Geom::Point val) {
    return x.interiorContains(val);
}

static bool wrap_interiorContains_ivl(Geom::Rect const &x, Geom::Rect val) {
    return x.interiorContains(val);
}

static void wrap_expandBy_pt(Geom::Rect &x, Geom::Point val) {
    x.expandBy(val);
}

static void wrap_expandBy(Geom::Rect &x, double val) {
    x.expandBy(val);
}

static void wrap_unionWith(Geom::Rect &x, Geom::Rect const &y) {
    x.unionWith(y);
}
static bool wrap_intersects(Geom::Rect const &x, Geom::Rect const &y) {
    return x.intersects(y);
}

void wrap_rect() {
    //TODO: fix overloads
    //def("unify", Geom::unify);
    def("union_list", Geom::union_list);
    //def("intersect", Geom::intersect);
    def("distanceSq", (double (*)( Geom::Point const&, Geom::Rect const&  ))Geom::distanceSq);
    def("distance", (double (*)( Geom::Point const&, Geom::Rect const&  ))Geom::distance);

    class_<Geom::Rect>("Rect", init<Geom::Interval, Geom::Interval>())
        .def(init<Geom::Point,Geom::Point>())
        .def(init<>())
        .def(init<Geom::Rect const &>())
        
        .def("__getitem__", python_getitem<Geom::Rect,Geom::Interval,2>)
    
        .def("min", &Geom::Rect::min)
        .def("max", &Geom::Rect::max)
        .def("corner", &Geom::Rect::corner)
        .def("top", &Geom::Rect::top)
        .def("bottom", &Geom::Rect::bottom)
        .def("left", &Geom::Rect::left)
        .def("right", &Geom::Rect::right)
        .def("width", &Geom::Rect::width)
        .def("height", &Geom::Rect::height)
        .def("dimensions", &Geom::Rect::dimensions)
        .def("midpoint", &Geom::Rect::midpoint)
        .def("area", &Geom::Rect::area)
        .def("maxExtent", &Geom::Rect::maxExtent)
        .def("contains", wrap_contains_coord)
        .def("contains", wrap_contains_ivl)
        .def("interiorContains", wrap_interiorContains_coord)
        .def("interiorContains", wrap_interiorContains_ivl)
        .def("intersects", wrap_intersects)
        .def("expandTo", &Geom::Rect::expandTo)
        .def("unionWith", &wrap_unionWith)
        // TODO: overloaded
        .def("expandBy", wrap_expandBy)
        .def("expandBy", wrap_expandBy_pt)
        
        .def(self * Geom::Affine())
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
