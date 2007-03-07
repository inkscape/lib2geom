/*
 * SVGPathParser - parse SVG path specifications
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

#include <vector>
#include <exception>
#include "../src/point.h"

class SVGPathParser {
public:
    SVGPathParser() {}
    virtual ~SVGPathParser() {}

    struct ParseError : public std::exception {
         char const *what() const throw() { return "parse error"; }
    };

    void parse(char const *str) throw(ParseError);

protected:
    virtual void moveTo(Geom::Point p) = 0;
    virtual void lineTo(Geom::Point p) = 0;
    virtual void curveTo(Geom::Point c0, Geom::Point c1, Geom::Point p) = 0;
    virtual void quadTo(Geom::Point c, Geom::Point p) = 0;
    virtual void arcTo(double rx, double ry, double angle,
                       bool large_arc, bool sweep, Geom::Point p) = 0;
    virtual void closePath() = 0;

private:
    Geom::Point _current;
    Geom::Point _initial;
    Geom::Point _cubic_tangent;
    Geom::Point _quad_tangent;

    void _reset() {
        _current = _initial = Geom::Point(0, 0);
        _quad_tangent = _cubic_tangent = Geom::Point(0, 0);
    }

    void _moveTo(Geom::Point p) {
        _quad_tangent = _cubic_tangent = _current = _initial = p;
        moveTo(p);
    }

    void _lineTo(Geom::Point p) {
        _quad_tangent = _cubic_tangent = _current = p;
        moveTo(p);
    }

    void _curveTo(Geom::Point c0, Geom::Point c1, Geom::Point p) {
        _quad_tangent = _current = p;
        _cubic_tangent = p + ( p - c1 );
        curveTo(c0, c1, p);
    }

    void _quadTo(Geom::Point c, Geom::Point p) {
        _cubic_tangent = _current = p;
        _quad_tangent = p + ( p - c );
        quadTo(c, p);
    }

    void _arcTo(double rx, double ry, double angle,
                bool large_arc, bool sweep, Geom::Point p)
    {
        _quad_tangent = _cubic_tangent = _current = p;
        arcTo(rx, ry, angle, large_arc, sweep, p);
    }

    void _closePath() {
        _quad_tangent = _cubic_tangent = _current = _initial;
        closePath();
    }
};


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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
