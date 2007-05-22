/*
 * callback interface for SVG path data
 *
 * Copyright 2007 MenTaLguY <mental@rydia.net>
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

#ifndef SEEN_SVG_PATH_H
#define SEEN_SVG_PATH_H

#include "path2.h"
#include <iterator>

namespace Geom {

class SVGPathSink {
public:
    virtual void moveTo(Point p) = 0;
    virtual void lineTo(Point p) = 0;
    virtual void curveTo(Point c0, Point c1, Point p) = 0;
    virtual void quadTo(Point c, Point p) = 0;
    virtual void arcTo(double rx, double ry, double angle,
                       bool large_arc, bool sweep, Point p) = 0;
    virtual void closePath() = 0;
    virtual void finish() = 0;
};

void output_svg_path(Path &path, SVGPathSink &sink);

template <typename OutputIterator>
class SVGPathGenerator : public SVGPathSink {
public:
    explicit SVGPathGenerator(OutputIterator out)
    : _in_path(false), _out(out) {}

    void moveTo(Point p) {
        finish();
        _path.start(p);
    }
//TODO: what if _in_path = false?
    void lineTo(Point p) {
        _path.appendNew<LineSegment>(p);
    }

    void curveTo(Point c0, Point c1, Point p) {
        _path.appendNew<CubicBezier>(c0, c1, p);
    }

    void quadTo(Point c, Point p) {
        _path.appendNew<QuadraticBezier>(c, p);
    }

    void arcTo(double rx, double ry, double angle,
               bool large_arc, bool sweep, Point p)
    {
        _path.appendNew<SVGEllipticalArc>(rx, ry, angle,
                                                 large_arc, sweep, p);
    }

    void closePath() {
        _path.close();
        finish();
    }

    void finish() {
        if (_in_path) {
            _in_path = false;
            *_out++ = _path;
            _path.clear();
            _path.close(false);
        }
    }

private:
    bool _in_path;
    OutputIterator _out;
    Path _path;
};

typedef std::back_insert_iterator<std::vector<Path> > iter;

class PathBuilder : public SVGPathGenerator<iter> {
private:
    std::vector<Path> _pathset;
public:
    PathBuilder() : SVGPathGenerator<iter>(iter(_pathset)) {}
    std::vector<Path> const &peek() const { return _pathset; }
};

/*
class PathBuilder {
private:
    SVGPathGenerator<iter> _gen;
    std::vector<Path> _pathset;
    double _tol;
    Point _cur;
public:
    explicit PathBuilder(double const &c = Geom_EPSILON) : _gen(iter(_pathset)) {
        _tol = c;
    }

    void moveTo(Point p) { _gen.moveTo(p); }
    void lineTo(Point p) { _gen.lineTo(p); }
    void curveTo(Point p0, Point p1, Point p2) { _gen.curveTo(p0, p1, p2); }
    void quadTo(Point p0, Point p1) { _gen.quadTo(p0, p1); }
    void arcTo(double rx, double ry, double angle,
               bool large_arc, bool sweep, Point p) {
        _gen.arcTo(rx, ry, angle, large_arc, sweep, p);
    }

    void moveToRel(Point p) { moveTo(_cur+p); }
    void lineToRel(Point p) { lineTo(_cur+p); }
    void curveToRel(Point p0, Point p1, Point p2) {
        curveTo(_cur+p0, _cur+p1, _cur+p2);
    }
    void quadToRel(Point p0, Point p1) {
        quadTo(_cur+p0, _cur+p1);
    }
    void arcToRel(double rx, double ry, double angle,
               bool large_arc, bool sweep, Point p) {
        arcTo(rx, ry, angle, large_arc, sweep, _cur+p);
    }

    void line(Point p0, Point p1) {
        if(LInfty(p0 - _cur) > _tol)
            moveTo(p0);
        lineTo(p1);
    }
    void curve(Point p0, Point p1, Point p2, Point p3) {
        if(LInfty(p0 - _cur) > _tol)
            moveTo(p0);
        curveTo(p1, p2, p3);
    }
    void quad(Point p0, Point p1, Point p2) {
        if(LInfty(p0 - _cur) > _tol)
            moveTo(p0);
        quadTo(p1, p2);
    }
    void arc(Point from, double rx, double ry, double angle,
               bool large_arc, bool sweep, Point p) {
        if(LInfty(from - _cur) > _tol)
            moveTo(from);
        arcTo(rx, ry, angle, large_arc, sweep, _cur+p);
    }

    std::vector<Path> const &peek() const { return _pathset; }
};
*/

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
