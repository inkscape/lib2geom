/*
 * Python bindings for lib2geom
 *
 * Copyright 2007 Aaron Spike <aaron@ekips.org>
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

#include "../../path2.h"
#include "../helpers.h"

#include "../../point.h"
#include "../../rect.h"
#include "../../multidim-sbasis.h"

using namespace boost::python;

/*
struct CurveWrap : Geom::Path2::Curve, wrapper<Geom::Path2::Curve>
{
    Geom::Point initialPoint(){return this->get_override("initialPoint")();}
    Geom::Point finalPoint(){return this->get_override("finalPoint")();}
    CurveWrap *duplicate(){return this->get_override("duplicate")();}
    Geom::Rect boundsFast(){return this->get_override("boundsFast")();}
    Geom::Rect boundsExact(){return this->get_override("boundsExact")();}
    void subdivide(Geom::Coord t, Geom::Path2::Curve& a, Geom::Path2::Curve& b){return this->get_override("subdivide")();}
    Geom::Point pointAndDerivativesAt(Geom::Coord t, unsigned n, Geom::Point *ds){return this->get_override("pointAndDerivativesAt")();}
    Geom::MultidimSBasis<2> sbasis(){return this->get_override("sbasis")();}
};
*/

BOOST_PYTHON_MODULE(_path2)
{
/*
    class_<CurveWrap>("Curve")
        .def("initalPoint", pure_virtual(&Geom::Path2::Curve::initialPoint))
        .def("finalPoint", pure_virtual(&CurveWrap::finalPoint))
        .def("duplicate", pure_virtual(&CurveWrap::duplicate))
        .def("boundsFast", pure_virtual(&CurveWrap::boundsFast))
        .def("boundsExact", pure_virtual(&CurveWrap::boundsExact))
        .def("subdivide", pure_virtual(&CurveWrap::subdivide))
        .def("pointAt", &CurveWrap::pointAt)
        .def("pointAndDerivativesAt", pure_virtual(&CurveWrap::PointAndDerivativesAt))
        .def("sbasis", pure_virtual(&CurveWrap::sbasis))
    ;
*/
    class_<Geom::Path2::Path>("Path")
        .def("empty", &Geom::Path2::Path::empty)
        .def("closed", &Geom::Path2::Path::closed)
        .def("close", &Geom::Path2::Path::closed)
    ;
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
