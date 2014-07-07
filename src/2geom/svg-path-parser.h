/**
 * \file
 * \brief  parse SVG path specifications
 *
 * Copyright 2007 MenTaLguY <mental@rydia.net>
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

#ifndef SEEN_SVG_PATH_PARSER_H
#define SEEN_SVG_PATH_PARSER_H

#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
#include <2geom/exception.h>
#include <2geom/point.h>
#include <2geom/path-sink.h>

namespace Geom {

class SVGPathParser {
public:
    SVGPathParser(PathSink &sink);

    void reset();

    void parse(char const *str);
    void parse(std::string const &s);

private:
    bool _absolute;
    Point _current;
    Point _initial;
    Point _cubic_tangent;
    Point _quad_tangent;
    std::vector<Coord> _params;
    PathSink &_sink;

    int cs;

    void _parse(char const *str);

    void _push(Coord value);
    Coord _pop();
    bool _pop_flag();
    Coord _pop_coord(Geom::Dim2 axis);
    Point _pop_point();
    void _moveTo(Point const &p);
    void _hlineTo(Point const &p);
    void _vlineTo(Point const &p);
    void _lineTo(Point const &p);
    void _curveTo(Point const &c0, Point const &c1, Point const &p);
    void _quadTo(Point const &c, Point const &p);
    void _arcTo(double rx, double ry, double angle,
                bool large_arc, bool sweep, Point const &p);
    void _closePath();
};

void parse_svg_path(char const *str, PathSink &sink);

inline void parse_svg_path(std::string const &str, PathSink &sink) {
    parse_svg_path(str.c_str(), sink);
}

inline PathVector parse_svg_path(char const *str) {
    PathVector ret;
    SubpathInserter iter(ret);
    PathIteratorSink<SubpathInserter> generator(iter);

    parse_svg_path(str, generator);
    return ret;
}

inline PathVector read_svgd_f(FILE * fi) {
    /// @bug The 10kB length limit should be removed
    char input[1024 * 10];
    fgets(input, 1024 * 10, fi);
    return parse_svg_path(input);
}

inline PathVector read_svgd(char const * name) {
    FILE* fi = fopen(name, "r");
    if(fi == NULL) throw(std::runtime_error("Error opening file"));
    PathVector out = read_svgd_f(fi);
    fclose(fi);
    return out;
}

}

#endif
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
