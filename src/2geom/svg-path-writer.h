/** @file
 * @brief Path sink which writes an SVG-compatible command string
 *//*
 * Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * 
 * Copyright 2014 Authors
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
 */

#ifndef SEEN_LIB2GEOM_SVG_PATH_WRITER_H
#define SEEN_LIB2GEOM_SVG_PATH_WRITER_H

#include <2geom/path-sink.h>
#include <sstream>

namespace Geom {

/** @brief SVG path data writer.
 * You can access the generated string by calling the str() method.
 */
class SVGPathWriter
    : public PathSink
{
public:
    SVGPathWriter();
    ~SVGPathWriter() {}

    void moveTo(Point const &p);
    void lineTo(Point const &p);
    void quadTo(Point const &c, Point const &p);
    void curveTo(Point const &c0, Point const &c1, Point const &p);
    void arcTo(double rx, double ry, double angle,
               bool large_arc, bool sweep, Point const &p);
    void closePath();
    void flush();

    void clear();
    void setPrecision(int prec);
    void setOptimize(bool opt) { _optimize = opt; }
    std::string str() const { return _s.str(); }

private:
    void _setCommand(char cmd);
    std::string _formatCoord(Coord par);

    std::ostringstream _s, _ns;
    std::vector<Coord> _current_pars;
    Point _subpath_start;
    Point _current;
    Point _quad_tangent;
    Point _cubic_tangent;
    Coord _epsilon;
    int _precision;
    bool _optimize;
    char _command;
};

} // namespace Geom

#endif // SEEN_LIB2GEOM_HEADER_H
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
