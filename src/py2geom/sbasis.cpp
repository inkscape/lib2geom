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
#include <boost/python/implicit.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include "py2geom.h"
#include "helpers.h"

#include "2geom/sbasis.h"
#include "2geom/sbasis-math.h"
#include "2geom/point.h"

using namespace boost::python;

Geom::SBasis (*truncate_sbasis)(Geom::SBasis const &, unsigned) = &Geom::truncate;
Geom::SBasis (*multiply_sbasis)(Geom::SBasis const &, Geom::SBasis const &) = &Geom::multiply;
Geom::SBasis (*integral_sbasis)(Geom::SBasis const &) = &Geom::integral;
Geom::SBasis (*derivative_sbasis)(Geom::SBasis const &) = &Geom::derivative;

Geom::Linear sbasis_getitem(Geom::SBasis const& p, int index)
{
    int D = p.size();
    if (index < 0)
    {
        index = D + index;
    }
    if ((index < 0) || (index > (D - 1))) {
        PyErr_SetString(PyExc_IndexError, "index out of range");
        boost::python::throw_error_already_set();
    }
    return p[index];
}

int sbasis_len(Geom::SBasis const& p)
{
    return p.size();
}

#include "2geom/sbasis-to-bezier.h"
#include "2geom/bezier.h"

Geom::Bezier sbasis_to_returned_bezier (Geom::SBasis const& sb, size_t sz = 0) {
    Geom::Bezier res;
    Geom::sbasis_to_bezier(res, sb, sz);
    return res;
}

/*object wrap_bounds_fast(Geom::SBasis const& sb) {
    Geom::OptInterval oi = bounds_fast(sb);
    return oi?object(*oi):object();
    }*/

template <typename T, typename target_type>
object wrap_bounds_fast(T const& sb) {
    target_type oi = bounds_fast(sb);
    return oi?object(*oi):object();
}


object wrap_bounds_exact(Geom::SBasis const& sb) {
    Geom::OptInterval oi = bounds_exact(sb);
    return oi?object(*oi):object();
}

object wrap_bounds_local(Geom::SBasis const& sb, Geom::Interval const & iv) {
    Geom::OptInterval oi = bounds_local(sb, iv);
    return oi?object(*oi):object();
}
void wrap_sbasis() {
    //sbasis.h

    def("shift", (Geom::SBasis (*)(Geom::SBasis const &a, int sh))&Geom::shift);
    def("truncate", truncate_sbasis);
    def("multiply", multiply_sbasis);
    def("compose", (Geom::SBasis (*) (Geom::SBasis const &, Geom::SBasis const &))&Geom::compose);
    def("integral", integral_sbasis);
    def("derivative", derivative_sbasis);
    def("min", (Geom::Piecewise<Geom::SBasis> (*)(Geom::SBasis const &, Geom::SBasis const & ))&Geom::min);
    def("sqrt", (Geom::SBasis (*)(Geom::SBasis const &, int ))&Geom::sqrt);
    def("reciprocal", (Geom::SBasis (*)(Geom::Linear const &, int ))&Geom::reciprocal);
    def("divide",(Geom::SBasis (*)(Geom::SBasis const &, Geom::SBasis const &, int )) &Geom::divide);
    def("inverse", (Geom::SBasis (*)(Geom::SBasis, int ))&Geom::inverse);
    //def("sin", (Geom::SBasis (*)(Geom::SBasis const &, int ))&Geom::sin);
    //def("cos", (Geom::SBasis (*)(Geom::SBasis const &, int ))&Geom::cos);
    def("reverse", (Geom::SBasis (*)(Geom::SBasis const &))&Geom::reverse);
    def("roots", (std::vector<double> (*)(Geom::SBasis const &))&Geom::roots);
    def("bounds_fast", &wrap_bounds_fast<Geom::SBasis, Geom::OptInterval>);
    def("bounds_exact", &wrap_bounds_exact);
    def("bounds_local", &wrap_bounds_local);
    def("sbasis_to_bezier", &::sbasis_to_returned_bezier);

    class_<Geom::SBasis>("SBasis", init<double>())
        .def(init<double, double>())
        .def(init<Geom::Linear>())
        .def(self_ns::str(self))
        //TODO: add important vector funcs
        .def("__getitem__", &sbasis_getitem)
        .def("__len__", &sbasis_len)

        .def("isZero", &Geom::SBasis::isZero)
        .def("isFinite", &Geom::SBasis::isFinite)
        .def("at0", (double (Geom::SBasis::*)() const) &Geom::SBasis::at0)
        .def("at1", (double (Geom::SBasis::*)() const) &Geom::SBasis::at1)
        .def("valueAt", &Geom::SBasis::valueAt)
        .def("toSBasis", &Geom::SBasis::toSBasis)

        .def("normalize", &Geom::SBasis::normalize)
        .def("tailError", &Geom::SBasis::tailError)
        .def("truncate", &Geom::SBasis::truncate)

        .def(self + self)
        .def(self - self)
        .def(self += self)
        .def(self -= self)

        .def(self + Geom::Linear())
        .def(self - Geom::Linear())
        .def(self += Geom::Linear())
        .def(self -= Geom::Linear())

        .def(self + float())
        .def(self - float())
        .def(self += float())
        .def(self -= float())

        .def(-self)
        .def(self * self)
        .def(self *= self)
        .def(self * float())
        .def(float() * self)
        .def(self / float())
        .def(self *= float())
        .def(self /= float())
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
