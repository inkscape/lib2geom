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

#include <2geom/svg-path-writer.h>
#include <iomanip>

namespace Geom {

SVGPathWriter::SVGPathWriter()
    : _precision(17)
{
    // always use C locale for number formatting
    _s.imbue(std::locale::classic());
    _s << std::setprecision(_precision);
    _s.unsetf(std::ios::floatfield);
}

void SVGPathWriter::moveTo(Point const &p)
{
    _s << "M " << p[X] << "," << p[Y];
}

void SVGPathWriter::hlineTo(Coord h)
{
    _s << " H " << h;
}
void SVGPathWriter::vlineTo(Coord v)
{
    _s << " V " << v;
}
void SVGPathWriter::lineTo(Point const &p)
{
    _s << " L " << p[X] << "," << p[Y];
}
void SVGPathWriter::curveTo(Point const &p1, Point const &p2, Point const &p3)
{
    _s << " C "
       << p1[X] << "," << p1[Y] << " "
       << p2[X] << "," << p2[Y] << " "
       << p3[X] << "," << p3[Y];
}
void SVGPathWriter::quadTo(Point const &c, Point const &p)
{
    _s << " Q "
       << c[X] << "," << c[Y] << " "
       << p[X] << "," << p[Y];
}
void SVGPathWriter::arcTo(double rx, double ry, double angle,
                          bool large_arc, bool sweep, Point const &p)
{
    _s << " A "
       << rx << "," << ry << " "
       << angle << " "
       << (large_arc ? "1 " : "0 ") << (sweep ? "1 " : "0 ")
       << p[X] << "," << p[Y];
}
void SVGPathWriter::closePath()
{
    _s << " z";
}
void SVGPathWriter::flush() {}

void SVGPathWriter::setPrecision(int prec)
{
    _precision = prec >= 0 ? 17 : prec;
    _s << std::setprecision(_precision);
}

} // namespace Geom

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
