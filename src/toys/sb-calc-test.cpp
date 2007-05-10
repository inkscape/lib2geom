/*
 *  sb-calculus.cpp - some std functions to work with (pw)s-basis
 *
 *  Authors:
 *   Jean-Francois Barraud
 *
 * Copyright (C) 2006-2007 authors
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

#include "s-basis.h"
#include "pw.h"
#include "sb-calculus.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "d2.h"

#include "path2.h"
#include "path-cairo.h"
#include "path2-builder.h"

#include "toy-framework.h"


#define ZERO 1e-3

using std::vector;
using namespace Geom;
using namespace std;

#include <stdio.h>
#include <math.h>


//-Plot---------------------------------------------------------------
static void plot(cairo_t* cr, SBasis const &f,double vscale=1,double a=0,double b=1){
    D2<SBasis> plot;
    plot[0]=SBasis(Linear(150+a*300,150+b*300));
    plot[1]=f*(-vscale);
    plot[1]+=300;
    cairo_md_sb(cr, plot);
    cairo_stroke(cr);
}

static void plot(cairo_t* cr, Piecewise<SBasis> const &f,double vscale=1){
    D2<Piecewise<SBasis> > plot;
    plot[1]=-f;
    plot[1]*=vscale;
    plot[1]+=300;

    plot[0].cuts.push_back(f.cuts.front());
    plot[0].cuts.push_back(f.cuts.back());
    plot[0].segs.push_back(Linear(150,450));

    cairo_d2_pw(cr, plot);
}

#define SIZE 4
#define NB_SEGS 1

class SbCalculusToy: public Toy {

  void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
      Piecewise<SBasis> f = Piecewise<SBasis>(Linear(-M_PI/2,2*M_PI));

      cairo_set_line_width (cr, .5);
      cairo_move_to(cr, Point(  0,300));
      cairo_line_to(cr, Point(600,300));
      cairo_move_to(cr, Point(210,  0));
      cairo_line_to(cr, Point(210,600));
      cairo_set_source_rgba (cr, 0.3, 0.3, 0.3, 1);
      cairo_stroke(cr);

      cairo_set_line_width (cr, 1.);

      cairo_set_source_rgba (cr, 1.0, 0.0, 0.0, 1);
      plot(cr,cos(f),50);
      cairo_stroke(cr);

      cairo_set_source_rgba (cr, 0.9, 0.0, 0.7, 1);
      plot(cr,sin(f),50);
      cairo_stroke(cr);

      cairo_set_source_rgba (cr, 0.3, 0., 0.3, 1);
      plot(cr,reciprocal(f,0.1),15);
      cairo_stroke(cr);

      cairo_set_source_rgba (cr, 0.0, 0.5, 0.5, 1);
      plot(cr,sqrt(f),50);
      cairo_stroke(cr);
      
      Toy::draw(cr, notify, width, height, save);
  }
  
public:
  SbCalculusToy(){
  }
};

int main(int argc, char **argv) {
    init(argc, argv, "sb-calculus", new SbCalculusToy);
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
// vim: filetype = cpp:expandtab:shiftwidth = 4:tabstop = 8:softtabstop = 4:encoding = utf-8:textwidth = 99 :
