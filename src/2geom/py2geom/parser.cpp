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

#include "py2geom.h"
#include "helpers.h"

#include "../svg-path.h"
#include "../svg-path-parser.h"


using namespace boost::python;

void (*parse_svg_path_str_sink) (char const *, Geom::SVGPathSink &) = &Geom::parse_svg_path;
std::vector<Geom::Path> (*parse_svg_path_str) (char const *) = &Geom::parse_svg_path;

class SVGPathSinkWrap: public Geom::SVGPathSink, public wrapper<Geom::SVGPathSink> {
    void moveTo(Geom::Point p) {this->get_override("moveTo")(p);}
    void hlineTo(Geom::Coord v) {this->get_override("hlineTo")(v);}
    void vlineTo(Geom::Coord v) {this->get_override("vlineTo")(v);}
    void lineTo(Geom::Point p) {this->get_override("lineTo")(p);}
    void curveTo(Geom::Point c0, Geom::Point c1, Geom::Point p) {this->get_override("curveTo")(c0, c1, p);}
    void quadTo(Geom::Point c, Geom::Point p) {this->get_override("quadTo")(c, p);}
    void arcTo(double rx, double ry, double angle, bool large_arc, bool sweep, Geom::Point p) {this->get_override("arcTo")(rx, ry, angle, large_arc, sweep, p);}
    void closePath() {this->get_override("closePath")();}
    void finish() {this->get_override("finish")();}
};

void wrap_parser() {
    def("parse_svg_path", parse_svg_path_str_sink);
    def("parse_svg_path", parse_svg_path_str);
    def("read_svgd", Geom::read_svgd);

    class_<SVGPathSinkWrap, boost::noncopyable>("SVGPathSink")
        .def("moveTo", pure_virtual(&Geom::SVGPathSink::moveTo))
        .def("hlineTo", pure_virtual(&Geom::SVGPathSink::hlineTo))
        .def("vlineTo", pure_virtual(&Geom::SVGPathSink::vlineTo))
        .def("lineTo", pure_virtual(&Geom::SVGPathSink::lineTo))
        .def("curveTo", pure_virtual(&Geom::SVGPathSink::curveTo))
        .def("quadTo", pure_virtual(&Geom::SVGPathSink::quadTo))
        .def("arcTo", pure_virtual(&Geom::SVGPathSink::arcTo))
        .def("closePath", pure_virtual(&Geom::SVGPathSink::closePath))
        .def("finish", pure_virtual(&Geom::SVGPathSink::finish))
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
