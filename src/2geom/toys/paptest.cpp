/*
 * A simple toy to test the path along path
 *
 * Copyright 2007 Johan Engelen
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
#include <2geom/path-sink.h>
#include <2geom/svg-path-parser.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/d2.h>
#include <2geom/piecewise.h>

Geom::Piecewise<Geom::D2<Geom::SBasis> >
doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > & pwd2_in, Geom::Piecewise<Geom::D2<Geom::SBasis> > & pattern)
{
    using namespace Geom;

    Piecewise<D2<SBasis> > uskeleton = arc_length_parametrization(pwd2_in, 2, .1);
    uskeleton = remove_short_cuts(uskeleton,.01);
    Piecewise<D2<SBasis> > n = rot90(derivative(uskeleton));
    n = force_continuity(remove_short_cuts(n,.1));

    D2<Piecewise<SBasis> > patternd2 = make_cuts_independent(pattern);
    Piecewise<SBasis> x = Piecewise<SBasis>(patternd2[0]);
    Piecewise<SBasis> y = Piecewise<SBasis>(patternd2[1]);
    Interval pattBnds = *bounds_exact(x);
    x -= pattBnds.min();
    Interval pattBndsY = *bounds_exact(y);
    y -= (pattBndsY.max()+pattBndsY.min())/2;

    int nbCopies = int(uskeleton.cuts.back()/pattBnds.extent());
    double scaling = 1;

    double pattWidth = pattBnds.extent() * scaling;

    if (scaling != 1.0) {
        x*=scaling;
    }

    double offs = 0;
    Piecewise<D2<SBasis> > output;
    for (int i=0; i<nbCopies; i++){
        output.concat(compose(uskeleton,x+offs)+y*compose(n,x+offs));
        offs+=pattWidth;
    }

    return output;
}

int main(int argc, char **argv) {
    if (argc > 1) {
        std::vector<Geom::Path> originald = Geom::parse_svg_path(&*argv[1]);
        Geom::Piecewise<Geom::D2<Geom::SBasis> > originaldpwd2;
        for (unsigned int i=0; i < originald.size(); i++) {
            originaldpwd2.concat( originald[i].toPwSb() );
        }

        std::vector<Geom::Path> pattern = Geom::parse_svg_path(&*argv[2]);
        Geom::Piecewise<Geom::D2<Geom::SBasis> > patternpwd2;
        for (unsigned int i=0; i < pattern.size(); i++) {
            patternpwd2.concat( pattern[i].toPwSb() );
        }
        
        doEffect_pwd2(originaldpwd2, patternpwd2);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
