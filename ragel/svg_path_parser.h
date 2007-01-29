/*
 * SVGPathParser - parse SVG path specifications
 *
 * Copyright 2007 MenTaLguY <mental@rydia.net>
 * Copyright 2007 Aaron Spike <aaron@ekips.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
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
