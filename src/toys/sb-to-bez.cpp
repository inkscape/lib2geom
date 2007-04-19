/* 
 * sb-to-bez Toy - Tests convertions from sbasis to cubic bezier.
 *
 * Copyright 2007 jf barraud.
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

// This is a draft test for convertion from sb to cubic bezier.
// I am not sure about what to do next; the improved method can be used
//   -to reduce the number of segments: same error, less segs.
//       this requires to find the best places to cut.
//   -to improve the approximation error on comparable segments.
// ...

#include "s-basis.h"
#include "sb-geometric.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "d2.h"

#include "path2.h"
#include "path-cairo.h"
#include "path2-builder.h"

#include "toy-framework.h"

#define ZERO 1e-7

using std::vector;
using namespace Geom;
using namespace std;

#include <stdio.h>
#include <gsl/gsl_poly.h>

void cairo_pw(cairo_t *cr, Piecewise<SBasis> p, double hscale=1., double vscale=1.) {
    for(int i = 0; i < p.size(); i++) {
        D2<SBasis> B;
        B[0] = Linear(150+p.cuts[i]*hscale, 150+p.cuts[i+1]*hscale);
        B[1] = Linear(450) - vscale*p[i];
        cairo_md_sb(cr, B);
    }
}

static void plot(cairo_t* cr, SBasis const &B,double vscale=1,double a=0,double b=1){
    D2<SBasis> plot;
    plot[0]=SBasis(Linear(150+a*300,150+b*300));
    plot[1]=-vscale*B;
    plot[1]+=300;
    cairo_md_sb(cr, plot);
    cairo_stroke(cr);
}
     
//==================================================================
static vector<double>  solve_poly (double a[],int deg){
    double tol=1e-7;
    vector<double> result;
    int i;

    i=deg;
    while( i>=0 && fabs(a[i])<tol ) i--;
    deg=i;
    
    double z[2*deg];
    
    gsl_poly_complex_workspace * w 
        = gsl_poly_complex_workspace_alloc (deg+1);
    
    gsl_poly_complex_solve (a, deg+1, w, z);
    
    gsl_poly_complex_workspace_free (w);
    
    for (i = 0; i < deg; i++){
        //printf ("z%d = %+.18f %+.18f\n", 
        //        i, z[2*i], z[2*i+1]);
        if (fabs(z[2*i+1])<tol){
            result.push_back(z[2*i]);
        }
    }
    return result;
}
//===================================================================================

static vector<Geom::Point> naive_sb_seg_to_bez(D2<Piecewise<SBasis> > const &M,double t0,double t1){
    vector<Geom::Point> result(4);
    D2<Piecewise<SBasis> > dM;
    for(int dim=0;dim<2;dim++){
        dM[dim]=derivative(M[dim]);
        result[0][dim]=M[dim](t0);
        result[1][dim]=M[dim](t0)+dM[dim](t0)*(t1-t0)/3;
        result[2][dim]=M[dim](t1)-dM[dim](t1)*(t1-t0)/3;
        result[3][dim]=M[dim](t1);
    }
    return(result);
}

static vector<Geom::Point> sb_seg_to_bez(D2<Piecewise<SBasis> > const &M,double t0,double t1){
    vector<Geom::Point> result(4);
    D2<double> M0,dM0,d2M0,M1,dM1,d2M1,A0,V0,A1,V1;
    D2<Piecewise<SBasis> > dM,d2M;
    for(int dim=0;dim<2;dim++){
        dM[dim]=derivative(M[dim]);
        d2M[dim]=derivative(dM[dim]);
        M0[dim]  =M[dim](t0);
        M1[dim]  =M[dim](t1);
        dM0[dim] =dM[dim](t0);
        dM1[dim] =dM[dim](t1);
        d2M0[dim]=d2M[dim](t0);
        d2M1[dim]=d2M[dim](t1);
        A0[dim]=M[dim](t0);
        A1[dim]=M[dim](t1);
    }
    //speed of bezier will be lambda0*dM0 and lambda1*dM1,
    //with lambda0 and lambda1 s.t. curvature at both ends is the same
    //as the curvature of the given curve.
    double lambda0,lambda1;
    double dM1xdM0=dM1[0]*dM0[1]-dM1[1]*dM0[0];
    if (fabs(dM1xdM0)<ZERO){
        lambda0= 6*((M1[0]-M0[0])*dM0[1]-(M1[1]-M0[1])*dM0[0]);
        lambda0=sqrt(lambda0/(d2M0[0]*dM0[1]-d2M0[1]*dM0[0]));
        lambda1=-6*((M1[0]-M0[0])*dM1[1]-(M1[1]-M0[1])*dM1[0]);
        lambda1=sqrt(lambda1/(d2M1[0]*dM1[1]-d2M1[1]*dM1[0]));
    }else{
        //solve:  lambda1 = a0 lambda0^2 + c0
        //        lambda0 = a1 lambda1^2 + c1
        double a0,c0,a1,c1;
        a0=-(d2M0[0]*dM0[1]-d2M0[1]*dM0[0])/2/dM1xdM0;
        c0= 3*((M1[0]-M0[0])*dM0[1]-(M1[1]-M0[1])*dM0[0])/dM1xdM0;
        a1=-(d2M1[0]*dM1[1]-d2M1[1]*dM1[0])/2/dM1xdM0;
        c1=-3*((M1[0]-M0[0])*dM1[1]-(M1[1]-M0[1])*dM1[0])/dM1xdM0;
        double lbda0_max;
        if (fabs(a0)<ZERO){
            lambda1=c0;
            lambda0= a1*lambda1*lambda1 + c1;
        }else if (fabs(a1)<ZERO){
            lambda0=c1;
            lambda1= a0*lambda0*lambda0 + c0;
        }else{
            //find lamda0 by solving a deg 4 equation d0+d1*X+...+d4*X^4=0
            double d0,d1,d2,d3,d4;
            d0=c1+a1*c0*c0;
            d1=-1;
            d2=2*a1*a0*c0;
            d3=0;
            d4=a1*a0*a0;
            double a[5]={d0,d1,d2,d3,d4};
            vector<double> solns=solve_poly(a,5);
            lambda0=lambda1=0;
            for (int i=0;i<solns.size();i++){
                double lbda0=solns[i];
                double lbda1=c0+a0*lbda0*lbda0;
                if (lbda0>=0. && lbda1>=0.){
                    lambda0=lbda0;
                    lambda1=lbda1;
                }
            }
        }
    }
    for(int dim=0;dim<2;dim++){
        result[0][dim]=A0[dim];
        result[1][dim]=A0[dim]+lambda0*dM0[dim]/3;
        result[2][dim]=A1[dim]-lambda1*dM1[dim]/3;
        result[3][dim]=A1[dim];
    }
    return(result);
}

//TODO: only works for t0=0 atm...
static double draw_non_parametric_approx(cairo_t *cr, 
                             D2<Piecewise<SBasis> > const &M, 
                             double t0,
                             double t1){
    assert(t0==0);
    vector<Geom::Point> bez_pts=sb_seg_to_bez(M,t0,t1);
    D2<SBasis>B=handles_to_sbasis<3>(bez_pts.begin());
    cairo_md_sb(cr, B);
    cairo_stroke(cr);

    Piecewise<SBasis>s_B=arc_length_sb(B);
    D2<Piecewise<SBasis> > Ms=arc_length_parametrization(M,3,.1);

    D2<Piecewise<SBasis> > D=compose(Ms,s_B);
    D[0] = D[0] - (Piecewise<SBasis>)B[0];
    D[1] = D[1] - (Piecewise<SBasis>)B[1];
    
    cairo_pw(cr, D[0],300,10);
    cairo_pw(cr, D[1],300,10);
    //cairo_pw(cr, s_B,300,1);
    cairo_stroke(cr);

    Interval bb;
    double err=0;
    bb=D[0].boundsFast();
    err+=std::max(fabs(bb.min()),fabs(bb.max()));
    bb=D[1].boundsFast();
    err+=std::max(fabs(bb.min()),fabs(bb.max()));

    return( err );   
}

static void draw_bez_approx_at(cairo_t *cr,D2<Piecewise<SBasis> > M,double t){
    vector<Geom::Point> bez_pts=sb_seg_to_bez(M,0,t);
    D2<SBasis> MM=handles_to_sbasis<3>(bez_pts.begin());
    cairo_md_sb(cr, MM);
    cairo_stroke(cr);
}


#define SIZE 6

class SbToBezierTester: public Toy {

  void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
    
      D2<SBasis> f;//=SBasis(Linear(0,.5));
      f=handles_to_sbasis<SIZE-1>(handles.begin());
      handles[SIZE][1]=450;
      handles[SIZE][0]=max(handles[SIZE][0],150.);
      handles[SIZE][0]=min(handles[SIZE][0],450.);
      //handles[SIZE+1][1]=450;
      //handles[SIZE+1][0]=max(handles[SIZE+1][0],150.);
      //handles[SIZE+1][0]=min(handles[SIZE+1][0],450.);
      double t0=0;//(handles[SIZE][0]-150)/300;
      double t1=(handles[SIZE][0]-150)/300;
      //if (t0>t1) {double temp=t0;t0=t1;t1=temp;}

      cairo_set_line_width (cr, 0.5);
      cairo_md_sb(cr, f);
      cairo_stroke(cr);
      if (t0==t1) return;//TODO: fix me...

      D2<Piecewise<SBasis> >g;

      g[0].cuts.push_back(0);
      g[0].cuts.push_back(1);
      g[0].segs.push_back(f[0]);

      g[1].cuts.push_back(0);
      g[1].cuts.push_back(1);
      g[1].segs.push_back(f[1]);

    
      //draw_bez_approx_at(cr,g,t0);      
      cairo_set_line_width (cr, 1);
      cairo_set_source_rgba (cr, 0., 0., 0.9, 1);
      double error=draw_non_parametric_approx(cr,g,t0,t1);

      cairo_set_line_width (cr, 1);
      cairo_set_source_rgba (cr, 0.9, 0., 0., 1);
      vector<Geom::Point> naive_bez_pts=naive_sb_seg_to_bez(g,0,t1);
      D2<SBasis> MM=handles_to_sbasis<3>(naive_bez_pts.begin());
      cairo_md_sb(cr, MM);
      cairo_stroke(cr);

      *notify << "Move handle 6 to set the segment to be approximated by cubic bezier.\n";
      *notify << " -red:  bezier approx derived from parametrization.\n";
      *notify << " -blue: bezier approx derived from curvature.\n";
      *notify << "      max distance (to original): "<<error<<"\n";

      Toy::draw(cr, notify, width, height, save);
  }
  
public:
  SbToBezierTester(){
    if(handles.empty()) {
      for(int i = 0; i < SIZE+1; i++)
	handles.push_back(Geom::Point(150+300*uniform(),150+300*uniform()));
    }
  }
};

int main(int argc, char **argv) {
    init(argc, argv, "bounds-test", new SbToBezierTester);
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
