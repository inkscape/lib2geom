/*
 * A simple toy to test the parser
 *
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


#include <iostream>
#include "svg-path.h"
#include "svg-path-parser.h"

class SVGPathTestPrinter : public Geom::SVGPathSink {
public:
    void moveTo(Geom::Point p) {
        std::cout << "M " << p << std::endl;
    }

    void lineTo(Geom::Point p) {
        std::cout << "L " << p << std::endl;
    }

    void curveTo(Geom::Point c0, Geom::Point c1, Geom::Point p) {
        std::cout << "C " << c0 << " " << c1 << " " << p << std::endl;
    }

    void quadTo(Geom::Point c, Geom::Point p) {
        std::cout << "Q " << c << " " << p << std::endl;
    }

    void arcTo(double rx, double ry, double angle,
               bool large_arc, bool sweep, Geom::Point p)
    {
        std::cout << "A " << rx << " " << ry << " " << angle << " " << large_arc << " " << sweep << " " << p << std::endl;
    }

    void closePath() {
        std::cout << "Z" << std::endl;
    }

    void finish() {
	;
    }

};


int main(int argc, char **argv) {
    if (argc > 1) {
        SVGPathTestPrinter sink;
        Geom::parse_svg_path(&*argv[1], sink);
    }
    return 0;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
