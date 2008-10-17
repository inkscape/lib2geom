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
#include <2geom/basic-intersection.h>

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

//===================================================================================

D2<SBasis>
naive_sb_seg_to_bez(Piecewise<D2<SBasis> > const &M,double t0,double t1){
    D2<SBasis> result;

    Piecewise<D2<SBasis> > dM = derivative(M); 
    Point M0  = M(t0);
    Point dM0 = dM(t0)*(t1-t0);
    Point M1  = M(t1);
    Point dM1 = dM(t1)*(t1-t0);
    for (unsigned dim=0; dim<2; dim++){
        result[dim].push_back(Linear(M0[dim],M1[dim]));
        result[dim].push_back(Linear(M0[dim]-M1[dim]+dM0[dim],-(M0[dim]-M1[dim]+dM1[dim])));
    }
    return result;
}

D2<SBasis>
sb_seg_to_bez(Piecewise<D2<SBasis> > const &M,double t0,double t1){
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
    
    std::vector<D2<SBasis> > candidates = cubics_fitting_curvature(M0,M1,dM0,dM1,d2M0,d2M1);
    if (candidates.size()==0){
        return D2<SBasis>(Linear(M0[X],M1[X]),Linear(M0[Y],M1[Y])) ;
    }
    double maxlength = -1;
    unsigned best = 0;
    for (unsigned i=0; i<candidates.size(); i++){
        double l = length(candidates[i]);
        if ( l < maxlength || maxlength < 0 ){
            maxlength = l;
            best = i;
        }
    }
    return candidates[best];
}
#include <2geom/sbasis-to-bezier.h>

/**
 * Compute the Hausdorf distance from A to B only.
 */
double hausdorfl2(cairo_t* cr, D2<SBasis>& A, D2<SBasis> const& B,
                 double m_precision,
                 double *a_t, double* b_t) {
    std::vector< std::pair<double, double> > xs;
    std::vector<Point> Az, Bz;
    sbasis_to_bezier (Az, A);
    sbasis_to_bezier (Bz, B);
    find_collinear_normal(xs, Az, Bz, m_precision);
    cairo_save(cr);
        cairo_set_line_width (cr, 0.3);
        cairo_set_source_rgba (cr, 0.0, 0.0, 0.7, 1);
        for (size_t i = 0; i < xs.size(); ++i)
        {
            Point At = A(xs[i].first);
            Point Bu = B(xs[i].second);
              cairo_move_to(cr, At);
              cairo_line_to(cr, Bu);
            draw_handle(cr, At);
            draw_handle(cr, Bu);

        }
        cairo_stroke(cr);
        cairo_restore(cr);
    double h_dist = 0, h_a_t = 0, h_b_t = 0;
    double dist = 0;
    Point Ax = A.at0();
    double t = Geom::nearest_point(Ax, B);
    dist = Geom::distance(Ax, B(t));
    if (dist > h_dist) {
        h_a_t = 0;
        h_b_t = t;
        h_dist = dist;
    }
    Ax = A.at1();
    t = Geom::nearest_point(Ax, B);
    dist = Geom::distance(Ax, B(t));
    if (dist > h_dist) {
        h_a_t = 1;
        h_b_t = t;
        h_dist = dist;
    }
    for (size_t i = 0; i < xs.size(); ++i)
    {
        Point At = A(xs[i].first);
        Point Bu = B(xs[i].second);
        double distAtBu = Geom::distance(At, Bu);
        t = xs[i].second;//Geom::nearest_point(At, B);
        dist = distAtBu;//Geom::distance(At, B(t));
        //FIXME: we might miss it due to floating point precision...
        if (dist >= distAtBu-.1 && distAtBu > h_dist) {
            h_a_t = xs[i].first;
            h_b_t = xs[i].second;
            h_dist = distAtBu;
        }
            
    }
    if(a_t) *a_t = h_a_t;
    if(b_t) *b_t = h_b_t;
    
    return h_dist;
}

void recursive_curvature_fitter(cairo_t* cr, D2<SBasis>  &f, double t0, double t1) {
      if (t0>=t1) return;//TODO: fix me...
      if (t0+0.001>=t1) return;//TODO: fix me...
      
      Piecewise<D2<SBasis> > g = Piecewise<D2<SBasis> >(f);
      D2<SBasis> k_bez = sb_seg_to_bez(g,t0,t1);
      double h_a_t = 0, h_b_t = 0;
      
      if(k_bez[0].size() > 1 and k_bez[1].size() > 1) {
          double h_dist = pseudo_hausdorf( k_bez, f, 1e-6, &h_a_t, &h_b_t);
          //printf("(%g %g), %g\n", t0, t1, h_dist);
          //double h_dist = hausdorfl2( cr, k_bez, f, 1e-6, &h_a_t, &h_b_t);
          //double h_dist = hausdorfl( k_bez, f, 1e-6, &h_a_t, &h_b_t);
          if(h_dist > 4) {
              recursive_curvature_fitter(cr, f, t0, (t0+t1)/2);
              recursive_curvature_fitter(cr, f, (t0+t1)/2, t1);
          } 
          else
          {
              Point At = k_bez(h_a_t);
              Point Bu = f(h_b_t);
              cairo_move_to(cr, At);
              cairo_line_to(cr, Bu);
              draw_handle(cr, At);
              draw_handle(cr, Bu);
              cairo_save(cr);
              cairo_set_line_width (cr, 0.3);
              cairo_set_source_rgba (cr, 0.7, 0.0, 0.0, 1);
              cairo_stroke(cr);
              cairo_restore(cr);
              cairo_md_sb(cr, k_bez);
              cairo_stroke(cr);
          }
      } else {
          recursive_curvature_fitter(cr, f, t0, (t0+t1)/2);
          recursive_curvature_fitter(cr, f, (t0+t1)/2, t1);
      } 
}

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

      cairo_set_source_rgba (cr, 0., 0., 0., 1);
      cairo_set_line_width (cr, 0.5);
      cairo_md_sb(cr, f);
      cairo_stroke(cr);
      if (t0==t1) return;//TODO: fix me...
      if(0) {
      Piecewise<D2<SBasis> > g = Piecewise<D2<SBasis> >(f);
      cairo_set_line_width (cr, 1);
      cairo_set_source_rgba (cr, 0., 0., 0.9, .7);
      double error=0;

      cairo_set_line_width (cr, 1);
      cairo_set_source_rgba (cr, 0.9, 0., 0., .7);
      D2<SBasis> naive_bez = naive_sb_seg_to_bez(g,0,t1);
      cairo_md_sb(cr, naive_bez);
      cairo_stroke(cr);

      adjuster2.pos[0]=150;
      adjuster2.pos[1]=std::min(std::max(adjuster2.pos[1],150.),450.);
      adjuster3.pos[0]=450;
      adjuster3.pos[1]=std::min(std::max(adjuster3.pos[1],150.),450.);

      double scale0=(450-adjuster2.pos[1])/150;
      double scale1=(450-adjuster3.pos[1])/150;

      cairo_set_line_width (cr, 1);
      cairo_set_source_rgba (cr, 0.7, 0., 0.7, .7);
      D2<SBasis> k_bez = sb_seg_to_bez(g,t0,t1);
      cairo_md_sb(cr, k_bez);
      cairo_stroke(cr);
      double h_a_t = 0, h_b_t = 0;
      
      double h_dist = pseudo_hausdorf( k_bez, f, 1e-6, &h_a_t, &h_b_t);
      {
          Point At = k_bez(h_a_t);
          Point Bu = f(h_b_t);
          cairo_move_to(cr, At);
          cairo_line_to(cr, Bu);
          draw_handle(cr, At);
          draw_handle(cr, Bu);
          cairo_save(cr);
          cairo_set_line_width (cr, 0.3);
          cairo_set_source_rgba (cr, 0.7, 0.0, 0.0, 1);
          cairo_stroke(cr);
          cairo_restore(cr);
      }
      *notify << "Move handle 6 to set the segment to be approximated by cubic bezier.\n";
      *notify << " -red:  bezier approx derived from parametrization.\n";
      *notify << " -blue: bezier approx derived from curvature.\n";
      *notify << "      max distance (to original): "<<h_dist<<"\n";
      }
      recursive_curvature_fitter(cr, f, 0, 1);
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
      handles.push_back(&adjuster2);
      adjuster3.pos = Geom::Point(450,300);
      handles.push_back(&adjuster3);
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
