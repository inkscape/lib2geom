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

class PathBuilder {
public:
    PathBuilder() : _current_path(NULL) {}

    void startPathRel(Point const &p0) { start_path(p0 + _current_point); }
    void startPath(Point const &p0) {
        _pathset.push_back(Geom::Path2::Path());
        _current_path = &_pathset.back();
        _initial_point = _current_point = p0;
    }

    void pushLineRel(Point const &p0) { pushLine(p0 + _current_point); }
    void pushLine(Point const &p1) {
        if (!_current_path) startPath(_current_point);
        _current_path->appendNew<LineSegment>(p1);
        _current_point = p1;
    }

    void pushLineRel(Point const &p0, Point const &p1) { pushLine(p0 + _current_point, p1 + _current_point); }
    void pushLine(Point const &p0, Point const &p1) {
        if(p0 != _current_point) startPath(p0);
        pushLine(p1);
    }

    void pushHorizontalRel(Coord y) { pushHorizontal(y + _current_point[1]); }
    void pushHorizontal(Coord y) {
        if (!_current_path) startPath(_current_point);
        pushLine(Point(_current_point[0], y));
    }

    void pushVerticalRel(Coord x) { pushVertical(x + _current_point[0]); }
    void pushvertical(Coord x) {
        if (!_current_path) startPath(_current_point);
        pushLine(Point(x, _current_point[1]));
    }

    void pushQuadraticRel(Point const &p1, Point const &p2) { pushQuadratic(p1 + _current_point, p2 + _current_point); }
    void pushQuadratic(Point const &p1, Point const &p2) {
        if (!_current_path) startPath(_current_point);
        _current_path->appendNew<QuadraticBezier>(p1, p2);
        _current_point = p2;
    }

    void pushQuadraticRel(Point const &p0, Point const &p1, Point const &p2) {
        pushQuadratic(p0 + _current_point, p1 + _current_point, p2 + _current_point);
    }
    void pushQuadratic(Point const &p0, Point const &p1, Point const &p2) {
        if(p0 != _current_point) startPath(p0);
        pushQuadratic(p1, p2);
    }

    void pushCubicRel(Point const &p1, Point const &p2, Point const &p3) {
        pushCubic(p1 + _current_point, p2 + _current_point, p3 + _current_point);
    }
    void pushCubic(Point const &p1, Point const &p2, Point const &p3) {
        if (!_current_path) startPath(_current_point);
        _current_path->appendNew<CubicBezier>(p1, p2, p3);
        _current_point = p3;
    }

    void pushCubicRel(Point const &p0, Point const &p1, Point const &p2, Point const &p3) {
        pushCubic(p0 + _current_point, p1 + _current_point, p2 + _current_point, p3 + _current_point);
    }
    void pushCubic(Point const &p0, Point const &p1, Point const &p2, Point const &p3) {
        if(p0 != _current_point) startPath(p0);
        pushCubic(p1, p2, p3);
    }

    void pushEllipseRel(Point const &radii, double rotation, bool large, bool sweep, Point const &end) {
        pushEllipse(radii, rotation, large, sweep, end + _current_point);
    }
    void pushEllipse(Point const &radii, double rotation, bool large, bool sweep, Point const &end) {
        if (!_current_path) startPath(_current_point);
        _current_path->appendNew<SVGEllipticalArc>(radii[0], radii[1], rotation, large, sweep, end);
        _current_point = end;
    }

    void pushEllipseRel(Point const &initial, Point const &radii, double rotation, bool large, bool sweep, Point const &end) {
        pushEllipse(initial + _current_point, radii, rotation, large, sweep, end + _current_point);
    }
    void pushEllipse(Point const &initial, Point const &radii, double rotation, bool large, bool sweep, Point const &end) {
        if(initial != _current_point) startPath(initial);
        pushEllipse(radii, rotation, large, sweep, end);
    }
    
    void closePath() {
        if (_current_path) {
            _current_path->close(true);
            _current_path = NULL;
        }
        _current_point = _initial_point = Point();
    }

    std::vector<Path> const &peek() const { return _pathset; }

private:
    std::vector<Path> _pathset;
    Path *_current_path;
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
