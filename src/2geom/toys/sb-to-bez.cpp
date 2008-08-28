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

// mainly experimental atm...
// do not expect to find anything understandable here atm. 

#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/bezier-to-sbasis.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

#define ZERO 1e-7

using std::vector;
using namespace Geom;
using namespace std;

#include <stdio.h>
#include <gsl/gsl_poly.h>

void cairo_pw(cairo_t *cr, Piecewise<SBasis> p, double hscale=1., double vscale=1.) {
    for(unsigned i = 0; i < p.size(); i++) {
        D2<SBasis> B;
        B[0] = Linear(150+p.cuts[i]*hscale, 150+p.cuts[i+1]*hscale);
        B[1] = Linear(450) - p[i]*vscale;
        cairo_md_sb(cr, B);
    }
}

//==================================================================
static vector<double>  solve_poly (double a[],unsigned deg){
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
    
    for (unsigned i = 0; i < deg; i++){
        //printf ("z%d = %+.18f %+.18f\n", 
        //        i, z[2*i], z[2*i+1]);
        if (fabs(z[2*i+1])<tol){
            result.push_back(z[2*i]);
        }
    }
    return result;
}
//===================================================================================

static vector<Geom::Point> naive_sb_seg_to_bez(Piecewise<D2<SBasis> > const &M,double t0,double t1){
    vector<Geom::Point> result(4);
    Piecewise<D2<SBasis> > dM = derivative(M);
    result[0]=M(t0);
    result[1]=M(t0)+dM(t0)*(t1-t0)/3;
    result[2]=M(t1)-dM(t1)*(t1-t0)/3;
    result[3]=M(t1);
    return(result);
}

static vector<Geom::Point> sb_seg_to_bez(Piecewise<D2<SBasis> > const &M,double t0,double t1){
    vector<Geom::Point> result(4);
    Point M0,dM0,d2M0,M1,dM1,d2M1,A0,V0,A1,V1;
    Piecewise<D2<SBasis> > dM,d2M;
        dM=derivative(M);
        d2M=derivative(dM);
        M0  =M(t0);
        M1  =M(t1);
        dM0 =dM(t0);
        dM1 =dM(t1);
        d2M0=d2M(t0);
        d2M1=d2M(t1);
        A0=M(t0);
        A1=M(t1);

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
        if (fabs(a0)<ZERO){
            lambda1=c0;
            lambda0= a1*lambda1*lambda1 + c1;
        }else if (fabs(a1)<ZERO){
            lambda0=c1;
            lambda1= a0*lambda0*lambda0 + c0;
        }else{
            //find lamda0 by solving a deg 4 equation d0+d1*X+...+d4*X^4=0
            double a[5]={c1+a1*c0*c0,-1,2*a1*a0*c0,0,a1*a0*a0};
            vector<double> solns=solve_poly(a,5);
            lambda0=lambda1=0;
            for (unsigned i=0;i<solns.size();i++){
                double lbda0=solns[i];
                double lbda1=c0+a0*lbda0*lbda0;
                if (lbda0>=0. && lbda1>=0.){
                    lambda0=lbda0;
                    lambda1=lbda1;
                }
            }
        }
    }
    for(unsigned dim=0;dim<2;dim++){
        result[0][dim]=A0[dim];
        result[1][dim]=A0[dim]+lambda0*dM0[dim]/3;
        result[2][dim]=A1[dim]-lambda1*dM1[dim]/3;
        result[3][dim]=A1[dim];
    }
    return(result);
}

/**
// this function is only used in commented code below

//TODO: only works for t0=0 atm...
static double draw_non_parametric_approx(cairo_t *cr, 
                             Piecewise<D2<SBasis> > const &M, 
                             double t0,
                             double t1){
    assert(t0==0);
    vector<Geom::Point> bez_pts=sb_seg_to_bez(M,t0,t1);
    D2<SBasis>B=handles_to_sbasis(bez_pts.begin(), 3);
    cairo_md_sb(cr, B);
    cairo_stroke(cr);

    Piecewise<SBasis>s_B=arcLengthSb(B);
    Piecewise<D2<SBasis> > Ms=arc_length_parametrization(M,3,.1);

    Piecewise<D2<SBasis> > D=compose(Ms,s_B) - Piecewise<D2<SBasis> >(B);
    
    //cairo_pw(cr, D[0],300,10);
    //cairo_pw(cr, D[1],300,10);
    //cairo_pw(cr, s_B,300,1);
    cairo_stroke(cr);

    //return( bounds_fast(D).maxExtent() );   
    return -1.;
}
**/

/**
// this function is only used in commented code below
static void draw_bez_approx_at(cairo_t *cr,Piecewise<D2<SBasis> > M,double t){
    vector<Geom::Point> bez_pts=sb_seg_to_bez(M,0,t);
    D2<SBasis> MM=handles_to_sbasis(bez_pts.begin(), 3);
    cairo_md_sb(cr, MM);
    cairo_stroke(cr);
}
**/

//--------------------------------------------------------------

//L2 proj of a general sb onto {f/ deg(f)=3, f(0)=f(1)=0}
static SBasis cubicL2Project(Piecewise<SBasis> const &f){
    SBasis e1, e2;
    e1.push_back(Linear(0));
    e1.push_back(Linear(sqrt(30.)));
    e2.push_back(Linear(0));
    e2.push_back(Linear(sqrt(210.),-sqrt(210.)));

    Piecewise<SBasis> prod;
    prod = integral(f*Piecewise<SBasis>(e1));
    double a1 = prod.segs.back().at1()-prod.segs.front().at0();
    prod = integral(f*Piecewise<SBasis>(e2));
    double a2 = prod.segs.back().at1()-prod.segs.front().at0();

    //return a1*e1+a2*e2;
    return e1*a1+e2*a2;
}

/**
// this function is only used in commented code below

//L2 approximation of M as b+n, where n is a section of the normal bundle.
// M is supposed to be parametrized by arc length.
static D2<SBasis> L2_proj(Piecewise<D2<SBasis> > const &M, 
                          D2<SBasis> b, 
                          unsigned depth=0){
    D2<SBasis> result, db=derivative(b);
    Piecewise<D2<SBasis> > udb = unitVector(db,.1);
    Piecewise<SBasis> sb = arcLengthSb(b);//TODO: don't compute unit vector twice!!
    Piecewise<D2<SBasis> > v = compose(M,sb)-Piecewise<D2<SBasis> >(b);
    Piecewise<SBasis> n = dot(v,rot90(udb));
    Piecewise<SBasis> h =divide( dot(v,Piecewise<D2<SBasis> >(db)), dot(db,db), .01, 3);
    n = compose(n,Piecewise<SBasis>(Linear(0,1)) - h);
   
    D2<Piecewise<SBasis> > nn = make_cuts_independent(n*rot90(udb));
    return (b+D2<SBasis>(cubicL2Project(nn[0]),cubicL2Project(nn[1])) );
}
**/


//#define SIZE 6

class SbToBezierTester: public Toy {
    //std::vector<Slider> sliders;
    PointSetHandle path_psh;
    PointHandle adjuster, adjuster2, adjuster3;

  void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
      cairo_save(cr);
      D2<SBasis> f;//=SBasis(Linear(0,.5));
      f = path_psh.asBezier();
      //f=handles_to_sbasis(handles.begin(), SIZE-1);
      adjuster.pos[1]=450;
      adjuster.pos[0]=std::max(adjuster.pos[0],150.);
      adjuster.pos[0]=std::min(adjuster.pos[0],450.);
      double t0=0;//(adjuster.pos[0]-150)/300;
      double t1=(adjuster.pos[0]-150)/300;
      //if (t0>t1) {double temp=t0;t0=t1;t1=temp;}

      cairo_set_line_width (cr, 0.5);
      cairo_md_sb(cr, f);
      cairo_stroke(cr);
      if (t0==t1) return;//TODO: fix me...

      Piecewise<D2<SBasis> > g = Piecewise<D2<SBasis> >(f);
      //draw_bez_approx_at(cr,g,t0);      
      cairo_set_line_width (cr, 1);
      cairo_set_source_rgba (cr, 0., 0., 0.9, .7);
      //double error=draw_non_parametric_approx(cr,g,t0,t1);
      double error=0;

      /*Piecewise<D2<SBasis> > ug = arc_length_parametrization(g);
      vector<Geom::Point> bez_pts_init=sb_seg_to_bez(g,t0,t1);
      D2<SBasis>b=handles_to_sbasis(bez_pts_init.begin(), 3);
      D2<SBasis>L2_approx=L2_proj(ug,b);
      cairo_set_source_rgba (cr, 0., 0.9, 0., .7);
      cairo_md_sb(cr, L2_approx);
      cairo_stroke(cr);*/

      cairo_set_line_width (cr, 1);
      cairo_set_source_rgba (cr, 0.9, 0., 0., .7);
      vector<Geom::Point> naive_bez_pts=naive_sb_seg_to_bez(g,0,t1);
      D2<SBasis> MM=handles_to_sbasis(naive_bez_pts.begin(), 3);
      cairo_md_sb(cr, MM);
      cairo_stroke(cr);

      adjuster2.pos[0]=150;
      adjuster2.pos[1]=std::min(std::max(adjuster2.pos[1],150.),450.);
      adjuster3.pos[0]=450;
      adjuster3.pos[1]=std::min(std::max(adjuster3.pos[1],150.),450.);

      double scale0=(450-adjuster2.pos[1])/150;
      double scale1=(450-adjuster3.pos[1])/150;

      cairo_set_line_width (cr, 1);
      cairo_set_source_rgba (cr, 0.7, 0., 0.7, .7);
      vector<Geom::Point> bez_pts=sb_seg_to_bez(g,t0,t1);
      bez_pts[1]=(1-scale0)*bez_pts[0]+scale0*bez_pts[1];
      bez_pts[2]=(1-scale1)*bez_pts[3]+scale1*bez_pts[2];
      D2<SBasis>human_approx=handles_to_sbasis(bez_pts.begin(), 3);
      cairo_md_sb(cr, human_approx);
      cairo_stroke(cr);

      *notify << "Move handle 6 to set the segment to be approximated by cubic bezier.\n";
      *notify << " -red:  bezier approx derived from parametrization.\n";
      *notify << " -blue: bezier approx derived from curvature.\n";
      *notify << "      max distance (to original): "<<error<<"\n";
      cairo_restore(cr);
      Toy::draw(cr, notify, width, height, save);
  }
  
public:
    SbToBezierTester() {
      //if(handles.empty()) {
      for(unsigned i = 0; i < 6; i++)
	path_psh.push_back(150+300*uniform(),150+300*uniform());
      handles.push_back(&path_psh);
      adjuster.pos = Geom::Point(150+300*uniform(),150+300*uniform());
      handles.push_back(&adjuster);
      adjuster2.pos = Geom::Point(150,300);
      //handles.push_back(&adjuster2);
      adjuster3.pos = Geom::Point(450,300);
      //handles.push_back(&adjuster3);
      //}
    //sliders.push_back(Slider(0.0, 1.0, 0.0, 0.0, "t"));
    //handles.push_back(&(sliders[0]));
  }
};

int main(int argc, char **argv) {
    init(argc, argv, new SbToBezierTester);
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
