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

//this a first try to define sqrt, cos, sin, etc...
//there are also d2 functions, but this part is still moving a lot.

#include "s-basis.h"
#include "sb-geometric.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
//#include "multidim-sbasis.h"
#include "d2.h"

#include "path2.h"
#include "path-cairo.h"
#include "path2-builder.h"

#include "toy-framework.h"

#include <time.h>

#define ZERO 1e-7

using std::vector;
using namespace Geom;
using namespace std;

#include <stdio.h>
#include <gsl/gsl_poly.h>


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


//-Sqrt---------------------------------------------------------------
static Piecewise<SBasis> sqrtOnDomain(Interval range, double tol=1e-7){
    Piecewise<SBasis> sqrt_fn;
    SBasis sqrt1_2=sqrt(Linear(1,2),2);
    double a=range.min(), b=range.max();
    int i0;
    if (a<=2*tol*tol){
        sqrt_fn.cuts.push_back(-1);
        sqrt_fn.push(Linear(0),0);
        a=2*tol*tol;
        i0=int(log(a)/log(2))-1;
        a=pow(2.,i0);
        sqrt_fn.push(Linear(0,sqrt(a)),a);
    }else{
        i0=int(log(a)/log(2))-1;
        a=pow(2.,i0);
        sqrt_fn.cuts.push_back(a);
    }  
    //double r=pow(2.,i0);
    while (a<b){
        sqrt_fn.push(sqrt1_2*sqrt(a),2*a);
        a*=2;
    }
//     for (int i=i0;2*r<=b;i++){
//         sqrt_fn.push(sqrt(r)*sqrt1_2,2*r);
//         r*=2;
//     }
    return(sqrt_fn);
}

static Piecewise<SBasis> my_sqrt(SBasis const &f, double tol=1e-2, int order=3){
    Piecewise<SBasis> sqrt_fn=sqrtOnDomain(boundsFast(f), tol);
    Piecewise<SBasis> result=compose(sqrt_fn,f);
    //TODO: define a truncated compose()!
    for (int k=0; k<result.segs.size(); k++){result.segs[k].truncate(order);}
    return(result);
}
static Piecewise<SBasis> my_sqrt(Piecewise<SBasis> f, double tol=1e-2, int order=3){
    Piecewise<SBasis> sqrt_fn=sqrtOnDomain(boundsFast(f), tol);
    Piecewise<SBasis> result=compose(sqrt_fn,f);
    //TODO: define a truncated compose()!
    for (int k=0; k<result.segs.size(); k++){result.segs[k].truncate(order);}
    return(result);
}

//-sin/cos--------------------------------------------------------------
static Piecewise<SBasis> cosOnDomain(Interval range){
    Piecewise<SBasis> cos_fn;
    SBasis cos0_90=cos(Linear(0,M_PI/2),3);
    int iMin=(int) floor(2*range.min()/M_PI);
    int iMax=(int) ceil(2*range.max()/M_PI);
    cos_fn.cuts.push_back(iMin*M_PI/2);
    for (int i=iMin; i<iMax; i++){
        double b=(i+1)*M_PI/2;
        //what does that stupid % operator for <0 numbers ?!?
        switch ((i%4>=0)?i%4:i%4+4) {
            case 0:
                cos_fn.push(cos0_90,b);
                break;
            case 1:
                cos_fn.push(-reverse(cos0_90),b);
                break;
            case 2:
                cos_fn.push(-cos0_90,b);
                break;
            case 3:
                cos_fn.push(reverse(cos0_90),b);
                break;
        }
    }
    return(cos_fn);
}

static Piecewise<SBasis> cos(SBasis const &f, double tol=1e-2){
    Piecewise<SBasis> cos_fn=cosOnDomain(boundsFast(f));
    return(compose(cos_fn,f));
}
static Piecewise<SBasis> cos(Piecewise<SBasis> f, double tol=1e-2){
    Piecewise<SBasis> cos_fn=cosOnDomain(boundsFast(f));
    return(compose(cos_fn,f));
}
static Piecewise<SBasis> sin(SBasis const &f, double tol=1e-2){
    SBasis g=f;
    g+=M_PI/2;
    Piecewise<SBasis> cos_fn=cosOnDomain(boundsFast(g));
    return(compose(cos_fn,g));
}
static Piecewise<SBasis> sin(Piecewise<SBasis> const &f, double tol=1e-2){
    Piecewise<SBasis> g=f;
    g+=M_PI/2;
    Piecewise<SBasis> cos_fn=cosOnDomain(boundsFast(g));
    return(compose(cos_fn,g));
}

//--x/y------------------------------------------------------------

static Piecewise<SBasis> ReciprocalOnDomain(Interval range, double tol=1e-7){
    Piecewise<SBasis> reciprocal_fn;
    SBasis reciprocal1_2=reciprocal(Linear(1,2),3);
    double a=range.min(), b=range.max();
    if (a*b<0){
        b=max(fabs(a),fabs(b));
        a=0;
    }else if (b<0){
        a=-range.max();
        b=-range.min();
    }

    if (a<=tol){
        reciprocal_fn.push_cut(0);
        int i0=(int) floor(log(tol)/log(2));
        a=pow(2.,i0);
        reciprocal_fn.push(Linear(1/a),a);
    }else{
        int i0=(int) floor(log(a)/log(2));
        a=pow(2.,i0);
        reciprocal_fn.cuts.push_back(a);
    }  

    while (a<b){
        reciprocal_fn.push(reciprocal1_2/a,2*a);
        a*=2;
    }
    if (range.min()<0 || range.max()<0){
        Piecewise<SBasis>reciprocal_fn_neg;
        //TODO: define reverse(pw<sb>);
        reciprocal_fn_neg.cuts.push_back(-reciprocal_fn.cuts.back());
        for (int i=0; i<reciprocal_fn.size(); i++){
            int idx=reciprocal_fn.segs.size()-1-i;
            reciprocal_fn_neg.push_seg(-reverse(reciprocal_fn.segs.at(idx)));
            reciprocal_fn_neg.push_cut(-reciprocal_fn.cuts.at(idx));
        }
        if (range.max()>0){
            reciprocal_fn_neg.concat(reciprocal_fn);
        }
        reciprocal_fn=reciprocal_fn_neg;
    }

    return(reciprocal_fn);
}

static Piecewise<SBasis> Reciprocal(SBasis const &f, double tol=1e-7, int order=3){
    Piecewise<SBasis> reciprocal_fn=ReciprocalOnDomain(boundsFast(f), tol);
    Piecewise<SBasis> result=compose(reciprocal_fn,f);
    //TODO: define a truncated compose() instead of computing useless coeffs!
    for (int k=0; k<result.segs.size(); k++){result.segs[k].truncate(order);}
    return(result);
}
static Piecewise<SBasis> Reciprocal(Piecewise<SBasis> f, double tol=1e-7, int order=3){
    Piecewise<SBasis> reciprocal_fn=ReciprocalOnDomain(boundsFast(f), tol);
    Piecewise<SBasis> result=compose(reciprocal_fn,f);
    //TODO: define a truncated compose()!
    for (int k=0; k<result.segs.size(); k++){result.segs[k].truncate(order);}
    return(result);
}


/*
hum! the D2 part is still moving a lot (and buggy).
 */
//--D2-------------------------------------------------------------
static Piecewise<D2<SBasis> > CutAtRoots(D2<SBasis> M, double tol=ZERO){
    vector<double> x_rts=roots(M[0]);
    vector<double> y_rts=roots(M[1]);
    vector<double> rts;
    
    //find common roots... is there an intersection function for vectors?
    int i=0,j=0;
    while ( i<x_rts.size() && j<y_rts.size() ){
        if (fabs(x_rts[i]-y_rts[j])<tol){
            printf("just found a singularity!! %f\n",x_rts[i]);
            rts.push_back(x_rts[i]);
            i+=1;
            j+=1;
        }else if (x_rts[i]<y_rts[j]){
            i+=1;
        }else if (x_rts[i]>y_rts[j]){
            j+=1;
        }
    }
    
    Piecewise<D2<SBasis> > result;
    if (rts.size()==0){
        result.push_cut(0);
        result.push(M,1);
        return result;
    }
    if (fabs(rts.front())<ZERO) {rts.erase(rts.begin());}
    if (fabs(rts.back()-1)<ZERO) {rts.pop_back();}
    result.push_cut(0);
    double t0=0;
    for (int i=0; i<rts.size(); i++){
        result.push( compose( M, Linear(t0,rts[i]) ), rts[i] );
        t0=rts[i];
    }
    result.push( compose( M, Linear(t0,1) ), 1 );
}

static SBasis devide_by_sk(SBasis const &a, int k) {
    if(k < 0) {
        return shift(a,-k);
    } else {
        SBasis c;
        if ( k>a.size()-1) return c;
        c.insert(c.begin(), a.begin()+k, a.end());
        return c;
    }
}

static SBasis divide_by_t0k(SBasis const &a, int k) {
    if(k < 0) {
        SBasis c = Linear(0,1);
        for (int i=0; i<-k; i++){
            c*=c;
        }
        c*=a;
        return(c);
    }else{
        SBasis c = Linear(1,0);
        for (int i=0; i<k; i++){
            c*=c;
        }
        c*=a;
        return(devide_by_sk(c,k));
    }
}

static SBasis divide_by_t1k(SBasis const &a, int k) {
    if(k < 0) {
        SBasis c = Linear(1,0);
        for (int i=0; i<-k; i++){
            c*=c;
        }
        c*=a;
        return(c);
    }else{
        SBasis c = Linear(0,1);
        for (int i=0; i<k; i++){
            c*=c;
        }
        c*=a;
        return(devide_by_sk(c,k));
    }
}

static D2<SBasis> RescaleForNonVanishingEnds(D2<SBasis> M, double tol=ZERO){
    int x_val = valuation(M[0],tol);
    int y_val = valuation(M[1],tol);
    //printf("valuations: %d, %d\n",x_val,y_val);
    M[0] = shift(M[0],-min(x_val,y_val));
    M[1] = shift(M[1],-min(x_val,y_val));
    while (fabs(M[0](0))<tol && fabs(M[1](0))<tol){
        //printf("divide by t\n");
        M[0] = divide_by_t0k(M[0],1);
        M[1] = divide_by_t0k(M[1],1);
    }
    while (M[0](1)==0 && M[1](1)==0){
        //printf("divide by 1-t\n");
        M[0] = divide_by_t1k(M[0],1);
        M[1] = divide_by_t1k(M[1],1);
    }
    return M;
}

static D2<Piecewise<SBasis> > 
NewUnitVector(D2<SBasis> V, double tol=ZERO, int order=3){
    //printf("\nNew unitVector...");
    Piecewise<D2<SBasis> >Vsegs=CutAtRoots(V,.01);
    //Piecewise<D2<SBasis> >Vsegs=Piecewise<D2<SBasis> >(V);
    D2<Piecewise<SBasis> > result;
    for (int i=0; i<Vsegs.size(); i++){
        //D2<SBasis>M = RescaleForNonVanishingEnds(Vsegs[i],.01);
        D2<SBasis>M = Vsegs[i];
    
        //printf("radius...");
        Piecewise<SBasis>  radius = my_sqrt(dot(M,M), tol, order);
        //printf("1/r...");
        Piecewise<SBasis>  radius_1 = Reciprocal(radius,.6,order);
        //printf("V/r (%d)...",radius_1.size());
        D2<Piecewise<SBasis> > unitM;
        for (int dim=0; dim<2; dim++){
            unitM[dim]=Piecewise<SBasis>(M[dim])*radius_1;
            //unitM[dim].truncate(order);
            //printf("...set dom...");
            unitM[dim].setDomain(Interval(Vsegs.cuts[i],Vsegs.cuts[i+1]));
            //printf("...push...");
            result[dim].concat(unitM[dim]);
        }
    }
    //printf("done\n");
    return result;
}


     
#define SIZE 4
#define NB_SEGS 1

class SbCalculusToy: public Toy {

  void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
    
      D2<Piecewise<SBasis> >g;
      Piecewise<SBasis> f;
      g[0].cuts.push_back(0);
      g[1].cuts.push_back(0);
      for (int i=0; i<NB_SEGS; i++){
          D2<SBasis> seg;
          seg=handles_to_sbasis<SIZE-1>(handles.begin()+i*SIZE);
          g[0].push(seg[0],i+1);
          g[1].push(seg[1],i+1);
      }

      cairo_set_line_width (cr, 0.5);
      cairo_set_source_rgba (cr, 0.9, 0., 0., 1);

//-- real->real functions------------------------------
      SBasis x=Linear(-M_PI/2,2*M_PI);
      f.cuts.push_back(0);
      f.push(x,1);
      plot(cr,cos(f),150);
      plot(cr,sin(f),150);
      plot(cr,Reciprocal(f,0.1),10);
      plot(cr,my_sqrt(f),50);

//-- real->point functions------------------------------
      D2<SBasis> gseg=sectionize(g)[0];
      D2<Piecewise<SBasis> > gsegpw;
      gsegpw[0]=Piecewise<SBasis>(gseg[0]);
      gsegpw[1]=Piecewise<SBasis>(gseg[1]);
      cairo_d2_pw(cr, gsegpw);

//       Piecewise<D2<SBasis> >Vsegs=CutAtRoots(derivative(gseg),.01);
//       for (int i=0; i<Vsegs.size(); i++){
//           draw_handle(cr, gseg(Vsegs.cuts[i]));
//       }


      D2<Piecewise<SBasis> >n = NewUnitVector(derivative(gseg));
      n*=50;
      n=rot90(n)+gsegpw;
      cairo_d2_pw(cr, n);

      cairo_stroke(cr);

      Toy::draw(cr, notify, width, height, save);
  }
  
public:
  SbCalculusToy(){
    if(handles.empty()) {
        for(int i = 0; i < SIZE; i++){
            for(int j = 0; j<NB_SEGS; j++){
                handles.push_back(Geom::Point(150+300*uniform(),
                                              150+300*uniform()));
            }
        }
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
