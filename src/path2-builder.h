/*
 * PathSetBuilder - build Path2s according using SVG/PDF like API
 *
 * Copyright 2006 MenTaLguY <mental@rydia.net>
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

#ifndef GEOM_PATH2_BUILDER_H
#define GEOM_PATH2_BUILDER_H

#include "path2.h"

namespace Geom {
namespace Path2 {

class PathSetBuilder {
public:
    PathSetBuilder() : _current_path(NULL) {}

    void start_path_rel(Point const &p0) { start_path(p0 + _current_point); }
    void start_path(Point const &p0) {
        _pathset.push_back(Geom::Path2::Path());
        _current_path = &_pathset.back();
        _initial_point = _current_point = p0;
    }

    void push_line_rel(Point const &p0) { push_line(p0 + _current_point); }
    void push_line(Point const &p1) {
        if (!_current_path) start_path(_current_point);
        _current_path->appendNew<LineSegment>(p1);
        _current_point = p1;
    }

    void push_line_rel(Point const &p0, Point const &p1) { push_line(p0 + _current_point, p1 + _current_point); }
    void push_line(Point const &p0, Point const &p1) {
        if(p0 != _current_point)
            start_path(p0);
        push_line(p1);
    }

    void push_horizontal_rel(Coord y) { push_horizontal(y + _current_point[1]); }
    void push_horizontal(Coord y) {
        if (!_current_path) start_path(_current_point);
        push_line(Point(_current_point[0], y));
    }

    void push_vertical_rel(Coord x) { push_vertical(x + _current_point[0]); }
    void push_vertical(Coord x) {
        if (!_current_path) start_path(_current_point);
        push_line(Point(x, _current_point[1]));
    }

    void push_quad_rel(Point const &p1, Point const &p2) { push_quad(p1 + _current_point, p2 + _current_point); }
    void push_quad(Point const &p1, Point const &p2) {
        if (!_current_path) start_path(_current_point);
        _current_path->appendNew<QuadraticBezier>(p1, p2);
        _current_point = p2;
    }

    void push_quad_rel(Point const &p0, Point const &p1, Point const &p2) {
        push_quad(p0 + _current_point, p1 + _current_point, p2 + _current_point);
    }
    void push_quad(Point const &p0, Point const &p1, Point const &p2) {
        if(p0 != _current_point)
            start_path(p0);
        push_quad(p1, p2);
    }

    void push_cubic_rel(Point const &p1, Point const &p2, Point const &p3) {
        push_cubic(p1 + _current_point, p2 + _current_point, p3 + _current_point);
    }
    void push_cubic(Point const &p1, Point const &p2, Point const &p3) {
        if (!_current_path) start_path(_current_point);
        _current_path->appendNew<CubicBezier>(p1, p2, p3);
        _current_point = p3;
    }

    void push_cubic_rel(Point const &p0, Point const &p1, Point const &p2, Point const &p3) {
        push_cubic(p0 + _current_point, p1 + _current_point, p2 + _current_point, p3 + _current_point);
    }
    void push_cubic(Point const &p0, Point const &p1, Point const &p2, Point const &p3) {
        if(p0 != _current_point)
            start_path(p0);
        push_cubic(p1, p2, p3);
    }

    void push_ellipse_rel(Point const &radii, double rotation, bool large, bool sweep, Point const &end) {
        push_ellipse(radii, rotation, large, sweep, end + _current_point);
    }
    void push_ellipse(Point const &radii, double rotation, bool large, bool sweep, Point const &end) {
        if (!_current_path) start_path(_current_point);
        _current_path->appendNew<SVGEllipticalArc>(radii[0], radii[1], rotation, large, sweep, end);
        _current_point = end;
    }

    void push_ellipse_rel(Point const &initial, Point const &radii, double rotation, bool large, bool sweep, Point const &end) {
        push_ellipse(initial + _current_point, radii, rotation, large, sweep, end + _current_point);
    }
    void push_ellipse(Point const &initial, Point const &radii, double rotation, bool large, bool sweep, Point const &end) {
        if(initial != _current_point)
            start_path(initial);
        push_ellipse(radii, rotation, large, sweep, end);
    }
    
    void close_path() {
        if (_current_path) {
            _current_path->close(true);
            _current_path = NULL;
        }
    }

    std::vector<Geom::Path2::Path> const &peek() const { return _pathset; }

private:
    std::vector<Geom::Path2::Path> _pathset;
    Geom::Path2::Path *_current_path;
    Point _current_point;
    Point _initial_point;
};

}
}

#endif 

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
