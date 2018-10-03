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

#include "2geom/path-sink.h"
#include "2geom/svg-path-parser.h"


using namespace boost::python;

void (*parse_svg_path_str_sink) (char const *, Geom::PathSink &) = &Geom::parse_svg_path;
Geom::PathVector (*parse_svg_path_str) (char const *) = &Geom::parse_svg_path;

void (Geom::PathSink::*feed_path)(Geom::Path const &) = &Geom::PathSink::feed;
void (Geom::PathSink::*feed_pathvector)(Geom::PathVector const &) = &Geom::PathSink::feed;

class PathSinkWrap: public Geom::PathSink, public wrapper<Geom::PathSink> {
    void moveTo(Geom::Point const &p) {this->get_override("moveTo")(p);}
    void lineTo(Geom::Point const &p) {this->get_override("lineTo")(p);}
    void curveTo(Geom::Point const &c0, Geom::Point const &c1, Geom::Point const &p) {this->get_override("curveTo")(c0, c1, p);}
    void quadTo(Geom::Point const &c, Geom::Point const &p) {this->get_override("quadTo")(c, p);}
    void arcTo(double rx, double ry, double angle, bool large_arc, bool sweep, Geom::Point const &p) {this->get_override("arcTo")(rx, ry, angle, large_arc, sweep, p);}
    bool backspace() {return this->get_override("backspace")();}
    void closePath() {this->get_override("closePath")();}
    void flush() {this->get_override("flush")();}
};

void wrap_parser() {
    def("parse_svg_path", parse_svg_path_str_sink);
    def("parse_svg_path", parse_svg_path_str);
    def("read_svgd", Geom::read_svgd);

    class_<PathSinkWrap, boost::noncopyable>("PathSink")
        .def("moveTo", pure_virtual(&Geom::PathSink::moveTo))
        .def("lineTo", pure_virtual(&Geom::PathSink::lineTo))
        .def("curveTo", pure_virtual(&Geom::PathSink::curveTo))
        .def("quadTo", pure_virtual(&Geom::PathSink::quadTo))
        .def("arcTo", pure_virtual(&Geom::PathSink::arcTo))
        .def("backspace", pure_virtual(&Geom::PathSink::backspace))
        .def("closePath", pure_virtual(&Geom::PathSink::closePath))
        .def("flush", pure_virtual(&Geom::PathSink::flush))
        .def("feed", feed_path)
        .def("feed", feed_pathvector)
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
