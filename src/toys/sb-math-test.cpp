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

#include "sb-math.h"
#include "pw.h"
#include "d2.h"
#include "s-basis.h"
#include "bezier-to-sbasis.h"

#include "path.h"
#include "path-cairo.h"

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
    plot[0].segs.push_back(Linear(150,450));

    cairo_d2_pw(cr, plot);

    for (int i=1; i<f.size(); i++){
        cairo_move_to(cr, Point(150+f.cuts[i]*300,300));
        cairo_line_to(cr, Point(150+f.cuts[i]*300,300-vscale*f.segs[i].at0()));
    }
}

double my_inv(double x){return (1./x);}

#define SIZE 4

class SbCalculusToy: public Toy {

  void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {

      //Let the user input sbasis coefs.
      SBasis B;
      for (int i=0;i<SIZE;i++){
          handles[i    ][0]=150+15*(i-SIZE);
          handles[i+SIZE][0]=450+15*(i+1);
          cairo_move_to(cr, Geom::Point(handles[i    ][0],150));
          cairo_line_to(cr, Geom::Point(handles[i    ][0],450));
          cairo_move_to(cr, Geom::Point(handles[i+SIZE][0],150));
          cairo_line_to(cr, Geom::Point(handles[i+SIZE][0],450));
      }
      cairo_move_to(cr, Geom::Point(0,300));
      cairo_line_to(cr, Geom::Point(600,300));
      
      cairo_set_line_width (cr, 1);
      cairo_set_source_rgba (cr, 0.2, 0.2, 0.2, 1);
      cairo_stroke(cr);
      
      for (int i=0;i<SIZE;i++){
          B.push_back(Linear(-(handles[i     ][1]-300)*pow(4.,i),
                             -(handles[i+SIZE][1]-300)*pow(4.,i) ));
      }
      Piecewise<SBasis> f = Piecewise<SBasis>(B);

//       cairo_set_line_width (cr, .5);
//       cairo_move_to(cr, Point(  0,300));
//       cairo_line_to(cr, Point(600,300));
//       cairo_move_to(cr, Point(210,  0));
//       cairo_line_to(cr, Point(210,600));
//       cairo_set_source_rgba (cr, 0.3, 0.3, 0.3, 1);
//       cairo_stroke(cr);

//      cairo_set_line_width (cr, 1.);
      
//plot relevant functions:

      cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, .8);
      plot(cr,f,1);
      cairo_stroke(cr);
      
      cairo_set_source_rgba (cr, 0.3, 0.3, 0.3,.8);
      plot(cr,abs(f),1);
      cairo_stroke(cr);

      cairo_set_source_rgba (cr, 0.3, 0.3, 0.3,.8);
      plot(cr,signSb(f),75);
      cairo_stroke(cr);

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
      
      Toy::draw(cr, notify, width, height, save);
  }
  
public:
  SbCalculusToy(){
      if(handles.empty()) {
          for(int i = 0; i < 2*SIZE; i++)
              handles.push_back(Geom::Point(0,150+150+uniform()*300*0));
      }
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
