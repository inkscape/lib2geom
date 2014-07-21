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
#include <glib.h>

namespace Geom {

SVGPathWriter::SVGPathWriter()
    : _precision(-1)
    , _optimize(false)
    , _command(0)
{
    // always use C locale for number formatting
    _ns.imbue(std::locale::classic());
    _ns.unsetf(std::ios::floatfield);
}

void SVGPathWriter::moveTo(Point const &p)
{
    _setCommand('M');
    _current_pars.push_back(p[X]);
    _current_pars.push_back(p[Y]);
    _current = p;
    _subpath_start = p;
    if (!_optimize) {
        flush();
    }
}
void SVGPathWriter::lineTo(Point const &p)
{
    if (_command != 'M' && _command != 'L') {
        _setCommand('L');
    }
    _current_pars.push_back(p[X]);
    _current_pars.push_back(p[Y]);
    _current = p;
    if (!_optimize) {
        flush();
    }
}
void SVGPathWriter::curveTo(Point const &p1, Point const &p2, Point const &p3)
{
    // TODO handle 'S' command
    _setCommand('C');
    _current_pars.push_back(p1[X]);
    _current_pars.push_back(p1[Y]);
    _current_pars.push_back(p2[X]);
    _current_pars.push_back(p2[Y]);
    _current_pars.push_back(p3[X]);
    _current_pars.push_back(p3[Y]);
    _current = p3;
    if (!_optimize) {
        flush();
    }
}
void SVGPathWriter::quadTo(Point const &c, Point const &p)
{
    _setCommand('Q');
    _current_pars.push_back(c[X]);
    _current_pars.push_back(c[Y]);
    _current_pars.push_back(p[X]);
    _current_pars.push_back(p[Y]);
    _current = p;
    if (!_optimize) {
        flush();
    }
}
void SVGPathWriter::arcTo(double rx, double ry, double angle,
                          bool large_arc, bool sweep, Point const &p)
{
    _setCommand('A');
    _current_pars.push_back(rx);
    _current_pars.push_back(ry);
    _current_pars.push_back(angle);
    _current_pars.push_back(large_arc ? 1. : 0.);
    _current_pars.push_back(sweep ? 1. : 0.);
    _current_pars.push_back(p[X]);
    _current_pars.push_back(p[Y]);
    _current = p;
    if (!_optimize) {
        flush();
    }
}
void SVGPathWriter::closePath()
{
    flush();
    if (_optimize) {
        _s << "z";
    } else {
        _s << " z";
    }
    _current = _subpath_start;
}
void SVGPathWriter::flush()
{
    if (_command == 0 || _current_pars.empty()) return;

    if (_optimize) {
        _s << _command;
    } else {
        if (_s.tellp() != 0) {
            _s << " ";
        }
        _s << _command;
    }

    char lastchar = _command;
    bool contained_dot = false;

    for (unsigned i = 0; i < _current_pars.size(); ++i) {
        // TODO: optimize the use of absolute / relative coords
        std::string cs = _formatCoord(_current_pars[i]);

        // Separator handling logic.
        // Floating point values can end with a digit or dot
        // and start with a digit, a plus or minus sign, or a dot.
        // The following cases require a separator:
        // * digit-digit
        // * digit-dot (only if the previous number didn't contain a dot)
        // * dot-digit
        if (_optimize) {
            // C++11: change to front()
            char firstchar = cs[0];
            if (g_ascii_isdigit(lastchar)) {
                if (g_ascii_isdigit(firstchar)) {
                    _s << " ";
                } else if (firstchar == '.' && !contained_dot) {
                    _s << " ";
                }
            } else if (lastchar == '.' && g_ascii_isdigit(firstchar)) {
                _s << " ";
            }
            _s << cs;

            // C++11: change to back()
            lastchar = cs[cs.length()-1];
            contained_dot = cs.find('.') != std::string::npos;
        } else {
            _s << " " << cs;
        }
    }
    _current_pars.clear();
    _command = 0;
}

void SVGPathWriter::clear()
{
    _s.clear();
    _s.str("");
    _ns.clear();
    _ns.str("");
    _command = 0;
    _current_pars.clear();
    _current = Point(0,0);
    _subpath_start = Point(0,0);
}

void SVGPathWriter::setPrecision(int prec)
{
    _precision = prec;
    _ns << std::setprecision(_precision);
}

void SVGPathWriter::_setCommand(char cmd)
{
    if (_command != 0 && _command != cmd) {
        flush();
    }
    _command = cmd;
}

std::string SVGPathWriter::_formatCoord(Coord par)
{
    std::string ret;
    if (_precision < 0) {
        // use dtostr, which guarantees roundtrip
        // TODO: use Grisu3 instead
        char buf[G_ASCII_DTOSTR_BUF_SIZE];
        g_ascii_dtostr(buf, G_ASCII_DTOSTR_BUF_SIZE, par);
        ret = buf;
    } else {
        // use the formatting stream with C locale
        _ns << par;
        ret = _ns.str();
        _ns.clear();
        _ns.str("");
    }
    return ret;
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
