/*
 *  sb-math-test.cpp - some std functions to work with (pw)s-basis
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

#include <2geom/sbasis-math.h>
#include <2geom/piecewise.h>
#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/bezier-to-sbasis.h>

#include <2geom/path.h>
#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>


#define ZERO 1e-3

using std::vector;
using namespace Geom;
using namespace std;

#include <stdio.h>
#include <math.h>

#include <toys/pwsbhandle.cpp>  // FIXME: This looks like it may give problems later, (including a .cpp file)

//-Plot---------------------------------------------------------------
static void plot(cairo_t* cr, double (*f)(double), Piecewise<SBasis> const &x, double vscale=1){
    int NbPts=40;
    for(int i=0; i<NbPts; i++){
        double t=double(i)/NbPts;
        t=x.cuts.front()*(1-t) + x.cuts.back()*t;
        draw_handle(cr, Point(150+i*300./NbPts,300-(*f)(x(t))*vscale));
        cairo_stroke(cr);
    }
}

static void plot(cairo_t* cr, Piecewise<SBasis> const &f,double vscale=1){
    D2<Piecewise<SBasis> > plot;
    plot[1]=-f;
    plot[1]*=vscale;
    plot[1]+=300;
    
    plot[0].cuts.push_back(f.cuts.front());
    plot[0].cuts.push_back(f.cuts.back());
    plot[0].segs.emplace_back(Linear(150,450));

    cairo_d2_pw_sb(cr, plot);

    for (unsigned i=1; i<f.size(); i++){
        cairo_move_to(cr, Point(150+f.cuts[i]*300,300));
        cairo_line_to(cr, Point(150+f.cuts[i]*300,300-vscale*f.segs[i].at0()));
    }
}

double my_inv(double x){return (1./x);}

#define SIZE 4

class SbCalculusToy: public Toy {
    PWSBHandle pwsbh;

  void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) override {

      //Let the user input sbasis coefs.
      cairo_move_to(cr, Geom::Point(0,300));
      cairo_line_to(cr, Geom::Point(600,300));
      
      cairo_set_line_width (cr, 1);
      cairo_set_source_rgba (cr, 0.2, 0.2, 0.2, 1);
      cairo_stroke(cr);
      
      Piecewise<SBasis> f = pwsbh.value(300);

      cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, .8);
      plot(cr,f,1);
      cairo_stroke(cr);
      
/*      cairo_set_source_rgba (cr, 0.3, 0.3, 0.3,.8);
      plot(cr,abs(f),1);
      cairo_stroke(cr);

      cairo_set_source_rgba (cr, 0.3, 0.3, 0.3,.8);
      plot(cr,signSb(f),75);
      cairo_stroke(cr);*/

//       cairo_set_source_rgba (cr, 0.3, 0.3, 0.3,.8);
//       plot(cr,maxSb(f, -f+50),1);
//       cairo_stroke(cr);

//       cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1);
//       plot(cr,sqrt(abs(f),.01,3),10);
//       plot(cr,&sqrt,abs(f),10);
//       cairo_stroke(cr);

//       cairo_set_source_rgba (cr, 1.0, 0.0, 0.0, 1);
//       plot(cr,cos(f,.01,3),75);
//       plot(cr,&cos,f,75);
//       cairo_stroke(cr);

//       cairo_set_source_rgba (cr, 0.9, 0.0, 0.7, 1);
//       plot(cr,sin(f,.01,3),75);
//       plot(cr,&sin,f,75);
//       cairo_stroke(cr);
      
      cairo_set_source_rgba (cr, 0.9, 0.0, 0.7, 1);
      plot(cr,divide(Linear(1),f,.01,3),2);
      plot(cr,&my_inv,f,10);
      cairo_stroke(cr);
      
      Toy::draw(cr, notify, width, height, save,timer_stream);
  }
  
public:
    SbCalculusToy() : pwsbh(4,1){
      for(int i = 0; i < 2*SIZE; i++)
          pwsbh.push_back(i*100,150+150+uniform()*300*0);
      handles.push_back(&pwsbh);
  }
};

int main(int argc, char **argv) {
    init(argc, argv, new SbCalculusToy);
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
