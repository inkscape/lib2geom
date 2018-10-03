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

#include "py2geom.h"

#include "2geom/transforms.h"

using namespace boost::python;

//TODO: properly wrap other transforms

void wrap_transforms() {
    class_<Geom::Affine>("Affine", init<double, double, double, double, double, double>())
        .def(init<>())
        .def(init<Geom::Rotate>())
        .def(init<Geom::Scale>())
        .def(init<Geom::Translate>())
        .def(self_ns::str(self))
        .add_property("xAxis",&Geom::Affine::xAxis,&Geom::Affine::setXAxis)
        .add_property("yAxis",&Geom::Affine::yAxis,&Geom::Affine::setYAxis)
        .add_property("translation",&Geom::Affine::translation,&Geom::Affine::setTranslation)
        .def("isTranslation", &Geom::Affine::isTranslation)
        .def("isRotation", &Geom::Affine::isRotation)
        .def("isScale", &Geom::Affine::isScale)
        .def("isUniformScale", &Geom::Affine::isUniformScale)
        .def("setIdentity", &Geom::Affine::setIdentity)
        .def("inverse", &Geom::Affine::inverse)
        .def("det", &Geom::Affine::det)
        .def("descrim2", &Geom::Affine::descrim2)
        .def("descrim", &Geom::Affine::descrim)
        .def("expansionX", &Geom::Affine::expansionX)
        .def("expansionY", &Geom::Affine::expansionY)
        .def(self * self)
        .def(self * other<Geom::Translate>())
        .def(self * other<Geom::Scale>())
        .def(self * other<Geom::Rotate>())
    ;

    class_<Geom::Scale>("Scale", init<double, double>())
        .def(self == self)
        .def(self != self)
        .def("inverse", &Geom::Scale::inverse)
        .def(Geom::Point() * self)
        .def(self * self)
        .def(self * Geom::Affine())
    ;

    class_<Geom::Translate>("Translate", init<double, double>())
        .def(init<Geom::Point>())
        .def(self == self)
        .def(self != self)
        .def("inverse", &Geom::Translate::inverse)
        .def(Geom::Point() * self)
        .def(self * self)
        .def(self * other<Geom::Rotate>())
        .def(self * other<Geom::Scale>())
    ;

    class_<Geom::Rotate>("Rotate", init<double>())
        .def(self == self)
        .def(self != self)
        .def("inverse", &Geom::Rotate::inverse)
        .def("from_degrees", &Geom::Rotate::from_degrees)
        .staticmethod("from_degrees")
        .def(Geom::Point() * self)
        .def(self * self)
        .def(Geom::Affine() * self)
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
