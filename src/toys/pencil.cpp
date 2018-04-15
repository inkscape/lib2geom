/* 
 * sb-to-bez Toy - Tests conversions from sbasis to cubic bezier.
 *
 * Copyright 2007 jf barraud.
 * 2008 njh
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

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>

#define ZERO 1e-7

using std::vector;
using namespace Geom;
using namespace std;

#include <cstdio>
#include <gsl/gsl_poly.h>

void cairo_pw(cairo_t *cr, Piecewise<SBasis> p, double hscale=1., double vscale=1.) {
    for(unsigned i = 0; i < p.size(); i++) {
        D2<SBasis> B;
        B[0] = Linear(150+p.cuts[i]*hscale, 150+p.cuts[i+1]*hscale);
        B[1] = Linear(450) - p[i]*vscale;
        cairo_d2_sb(cr, B);
    }
}

//===================================================================================

D2<SBasis>
naive_sb_seg_to_bez(Piecewise<D2<SBasis> > const &M,double t0,double t1){

    Piecewise<D2<SBasis> > dM = derivative(M); 
    Point M0  = M(t0);
    Point dM0 = dM(t0)*(t1-t0);
    Point M1  = M(t1);
    Point dM1 = dM(t1)*(t1-t0);
    D2<SBasis> result;
    for (unsigned dim=0; dim<2; dim++){
        SBasis r(2, Linear());
        r[0] = Linear(M0[dim],M1[dim]);
        r[1] = Linear(M0[dim]-M1[dim]+dM0[dim],-(M0[dim]-M1[dim]+dM1[dim]));
        result[dim] = r;
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
        return D2<SBasis>(SBasis(M0[X],M1[X]),SBasis(M0[Y],M1[Y])) ;
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

int goal_function_type = 0;

double goal_function(Piecewise<D2<SBasis> >const &A, 
                     Piecewise<D2<SBasis> >const&B) {
    if(goal_function_type) {
        OptInterval bnds = bounds_fast(dot(derivative(A), rot90(derivative(B))));
        //double h_dist = bnds.dimensions().length();
//0 is in the rect!, TODO:gain factor ~2 for free.
// njh: not really, the benefit is actually rather small.
        double h_dist = 0;
        if(bnds)
            h_dist = bnds->extent();
        return h_dist ;
    } else {
        Rect bnds = *bounds_fast(A - B);
        return max(bnds.min().length(), bnds.max().length());
    }  
}

int recursive_curvature_fitter(cairo_t* cr, Piecewise<D2<SBasis> > const &f, double t0, double t1, double precision) {
      if (t0>=t1) return 0;//TODO: fix me...
      if (t0+0.001>=t1) return 0;//TODO: fix me...
      
      //TODO: don't re-compute derivative(f) at each try!!
      D2<SBasis> k_bez = sb_seg_to_bez(f,t0,t1);      

      if(k_bez[0].size() > 1 and k_bez[1].size() > 1) {
          Piecewise<SBasis> s = arcLengthSb(k_bez);
          s *= (t1-t0)/arcLengthSb(k_bez).segs.back().at1();
          s += t0;
          double h_dist = goal_function(compose(f,s), Piecewise<D2<SBasis> >(k_bez));
          if(h_dist < precision) {
              cairo_save(cr);
              cairo_set_line_width (cr, 0.93);
              cairo_set_source_rgba (cr, 0.7, 0.0, 0.0, 1);
              draw_handle(cr, k_bez.at0());
              cairo_d2_sb(cr, k_bez);
              cairo_stroke(cr);
              cairo_restore(cr);
              return 1;
          }
      }
      //TODO: find a better place where to cut (at the worst fit?).
      return recursive_curvature_fitter(cr, f, t0, (t0+t1)/2, precision) +
          recursive_curvature_fitter(cr, f, (t0+t1)/2, t1, precision);
}

double single_curvature_fitter(Piecewise<D2<SBasis> > const &f, double t0, double t1) {
      if (t0>=t1) return 0;//TODO: fix me...
      if (t0+0.001>=t1) return 0;//TODO: fix me...
      
      D2<SBasis> k_bez = sb_seg_to_bez(f,t0,t1);      

      if(k_bez[0].size() > 1 and k_bez[1].size() > 1) {
          Piecewise<SBasis> s = arcLengthSb(k_bez);
          s *= (t1-t0)/arcLengthSb(k_bez).segs.back().at1();
          s += t0;
          return goal_function(compose(f,s), Piecewise<D2<SBasis> >(k_bez));
      }
      return 1e100;
}

struct quadratic_params
{
    Piecewise<D2<SBasis> > const *f;
    double t0, precision;
};
     

double quadratic (double x, void *params) {
    struct quadratic_params *p 
        = (struct quadratic_params *) params;
     
    return single_curvature_fitter(*p->f, p->t0, x) - p->precision;
}
     
#include <cstdio>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_roots.h>
     
     
int sequential_curvature_fitter(cairo_t* cr, Piecewise<D2<SBasis> > const &f, double t0, double t1, double precision) {
    if(t0 >= t1) return 0;
    
    double r = t1;
    if(single_curvature_fitter(f, t0, t1) > precision) {
        int status;
        int iter = 0, max_iter = 100;
        const gsl_root_fsolver_type *T;
        gsl_root_fsolver *s;
        gsl_function F;
        struct quadratic_params params = {&f, t0, precision};
     
        F.function = &quadratic;
        F.params = &params;
     
        T = gsl_root_fsolver_brent;
        s = gsl_root_fsolver_alloc (T);
        gsl_root_fsolver_set (s, &F, t0, t1);
     
        do
        {
            iter++;
            status = gsl_root_fsolver_iterate (s);
            r = gsl_root_fsolver_root (s);
            double x_lo = gsl_root_fsolver_x_lower (s);
            double x_hi = gsl_root_fsolver_x_upper (s);
            status = gsl_root_test_interval (x_lo, x_hi,
                                             0, 0.001);
     
     
        }
        while (status == GSL_CONTINUE && iter < max_iter);
    
        double x_lo = gsl_root_fsolver_x_lower (s);
        double x_hi = gsl_root_fsolver_x_upper (s);
        printf ("%5d [%.7f, %.7f] %.7f %.7f\n",
                iter, x_lo, x_hi,
                r, 
                x_hi - x_lo);
        gsl_root_fsolver_free (s);
    }    
    D2<SBasis> k_bez = sb_seg_to_bez(f,t0,r);      
    
    cairo_save(cr);
    cairo_set_line_width (cr, 0.93);
    cairo_set_source_rgba (cr, 0.7, 0.0, 0.0, 1);
    draw_handle(cr, k_bez.at0());
    cairo_d2_sb(cr, k_bez);
    cairo_stroke(cr);
    cairo_restore(cr);
    
    if(r < t1)
        return sequential_curvature_fitter(cr, f, r, t1, precision) + 1;
    return 1;
}


class SbToBezierTester: public Toy {
    //std::vector<Slider> sliders;
    PointHandle adjuster, adjuster2, adjuster3;
    std::vector<Toggle> toggles;
    Piecewise<D2<SBasis > > stroke;

  void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) override {
      cairo_save(cr);

      cairo_set_source_rgba (cr, 0., 0., 0., 1);
      cairo_set_line_width (cr, 0.5);
      
      if(!mouses.empty()) {
	  cairo_move_to(cr, mouses[0]);
	  for(auto mouse : mouses) {
	      cairo_line_to(cr, mouse);
	  }
	  cairo_stroke(cr);
      }
      adjuster2.pos[0]=150;
      adjuster2.pos[1]=std::min(std::max(adjuster2.pos[1],150.),450.);
      cairo_move_to(cr, 150, 150);
      cairo_line_to(cr, 150, 450);
      cairo_stroke(cr);
      ostringstream val_s;
      double scale0=(450-adjuster2.pos[1])/300;
      double curve_precision = pow(10, scale0*5-2);
      val_s << curve_precision;
      draw_text(cr, adjuster2.pos, val_s.str().c_str());
      
      
      Piecewise<D2<SBasis> > f_as_pw = stroke;
      cairo_pw_d2_sb(cr, f_as_pw);
      cairo_stroke(cr);
      if(!stroke.empty()) {
	  f_as_pw = arc_length_parametrization(f_as_pw);
	  int segs = 0;
	  goal_function_type = toggles[1].on;
	  if(toggles[0].on)
	      segs = sequential_curvature_fitter(cr, f_as_pw, 0, f_as_pw.cuts.back(), curve_precision);
	  else {
	      segs = recursive_curvature_fitter(cr, f_as_pw, 0, f_as_pw.cuts.back(),curve_precision);
	  }
	  *notify << "      total segments: "<< segs <<"\n";
      }
      cairo_restore(cr);
      Point p(25, height - 50), d(50,25);
      toggles[0].bounds = Rect(p,     p + d);
      p+= Point(75, 0);
      toggles[1].bounds = Rect(p,     p + d);
      draw_toggles(cr, toggles);
      Toy::draw(cr, notify, width, height, save,timer_stream);
  }
  
public:
    void key_hit(GdkEventKey *e) override {
        if(e->keyval == 's') toggles[0].toggle();
        redraw();
    }
    vector<Point> mouses;
    int mouse_drag;
    
    void mouse_pressed(GdkEventButton* e) override {
        toggle_events(toggles, e);
	Toy::mouse_pressed(e);
	if(!selected) {
	    mouse_drag = 1;
	    mouses.clear();
	}
    }
    
    
    void mouse_moved(GdkEventMotion* e) override {
	if(mouse_drag) {
	    mouses.emplace_back(e->x, e->y);
	    redraw();
	} else {
	    Toy::mouse_moved(e);
	}
    }

    void mouse_released(GdkEventButton* e) override {
	mouse_drag = 0;
	stroke.clear();
	stroke.push_cut(0);
	Path pth;
	for(unsigned i = 2; i < mouses.size(); i+=2) {
	    pth.append(QuadraticBezier(mouses[i-2], mouses[i-1], mouses[i]));
	}
	stroke = pth.toPwSb();
	Toy::mouse_released(e);
    }

    SbToBezierTester() {
      adjuster.pos = Geom::Point(150+300*uniform(),150+300*uniform());
      handles.push_back(&adjuster);
      adjuster2.pos = Geom::Point(150,300);
      handles.push_back(&adjuster2);
      adjuster3.pos = Geom::Point(450,300);
      handles.push_back(&adjuster3);
      toggles.emplace_back("Seq", false);
      toggles.emplace_back("Linfty", true);
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
