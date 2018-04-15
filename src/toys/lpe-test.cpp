/**
 * \file lpe-test.cpp
 * \brief Example file showing how to write an Inkscape Live Path Effect toy.
 */
/*
 * Copyright 2009  Johan Engelen <goejendaagh@zonnet.nl>
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

#include <toys/lpe-framework.h>

// This is usually very bad practice: don't do it in your LPE
using std::vector;
using namespace Geom;
using namespace std;

class LPETest: public LPEToy {
public:
    LPETest() {
        concatenate_before_pwd2 = false;
    }

    Geom::Piecewise<Geom::D2<Geom::SBasis> >
    doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in) override
    {
        using namespace Geom;

        Piecewise<D2<SBasis> > pwd2_out = pwd2_in;

        Point vector(50,100);
        // generate extrusion bottom: (just a copy of original path, displaced a bit)
        pwd2_out.concat( pwd2_in + vector );

        // generate connecting lines (the 'sides' of the extrusion)
        Path path(Point(0.,0.));
        path.appendNew<Geom::LineSegment>( vector );
        Piecewise<D2<SBasis> > connector = path.toPwSb();
        // connecting lines should be put at cusps
        Piecewise<D2<SBasis> > deriv = derivative(pwd2_in);
        std::vector<double> cusps; // = roots(deriv);
        for (double cusp : cusps) {
            pwd2_out.concat( connector + pwd2_in.valueAt(cusp) );
        }
        // connecting lines should be put where the tangent of the path equals the extrude_vector in direction
        std::vector<double> rts = roots(dot(deriv, rot90(vector)));
        for (double rt : rts) {
            pwd2_out.concat( connector + pwd2_in.valueAt(rt) );
        }

        return pwd2_out;
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new LPETest);
    return 0;
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
//vim:expandtab:shiftwidth = 4:tabstop = 8:softtabstop = 4:encoding = utf-8:textwidth = 99 :


