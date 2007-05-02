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

//TODO: define mapToDomain(n,t) or something...
//TODO: define D2<pw<sb> pwd2_to_d2pw(pw<D2<sb>>) or something...


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
    while (a<b){
        sqrt_fn.push(sqrt1_2*sqrt(a),2*a);
        a*=2;
    }
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


//=================================================================
/*
hum! the D2 part is still moving a lot (and buggy).
 */
//--D2-------------------------------------------------------------

static D2<Piecewise<SBasis> > 
pwd2_to_d2pw(Piecewise<D2<SBasis> > M){
    D2<Piecewise<SBasis> > result;
    for (int dim=0; dim<2; dim++){
        result[dim].push_cut(M.cuts.front());
        for (int i=0; i<M.size(); i++){
            result[dim].push(M[i][dim],M.cuts[i+1]);
        }
    }
    return result;
}
//=================================================================
static Piecewise<D2<SBasis> >
derivative(Piecewise<D2<SBasis> > const &M){
    Piecewise<D2<SBasis> > result;
    if (M.empty()) return M;
    result.push_cut(M.cuts[0]);
    for (int i=0; i<M.size(); i++){
        result.push(derivative(M.segs[i]),M.cuts[i+1]);
    }
    return result;
}
static Piecewise<D2<SBasis> >
rot90(Piecewise<D2<SBasis> > const &M){
    Piecewise<D2<SBasis> > result;
    if (M.empty()) return M;
    result.push_cut(M.cuts[0]);
    for (int i=0; i<M.size(); i++){
        result.push(rot90(M[i]),M.cuts[i+1]);
    }
    return result;
}

//=================================================================

static vector<double> 
intersect(vector<double> const &a, vector<double> const &b, double tol=0.){
    vector<double> inter;
    int i=0,j=0;
    while ( i<a.size() && j<b.size() ){
        if (fabs(a[i]-b[j])<tol){
            inter.push_back(a[i]);
            i+=1;
            j+=1;
        }else if (a[i]<b[j]){
            i+=1;
        }else if (a[i]>b[j]){
            j+=1;
        }
    }
    return inter;
}


static Piecewise<D2<SBasis> > 
sectionizeAtCutsAndRoots(D2<Piecewise<SBasis> > const &M_in){
    D2<Piecewise<SBasis> > M=M_in;
    vector<double> x_rts=roots(M[0]);
    vector<double> y_rts=roots(M[1]);
    vector<double> rts=intersect(x_rts, y_rts, ZERO);
    M[0] = partition(M[0],rts);
    return sectionize(M);
}

static Piecewise<D2<SBasis> > 
cutAtRoots(Piecewise<D2<SBasis> > const &M){
    vector<double> rts;
    for (int i=0; i<M.size(); i++){
        vector<double> seg_rts = roots((M.segs[i])[0]);
        seg_rts = intersect(seg_rts, roots((M.segs[i])[1]), ZERO);
        Linear mapToDom = Linear(M.cuts[i],M.cuts[i+1]);
        for (int r=0; r<seg_rts.size(); r++){
            seg_rts[r]= mapToDom(seg_rts[r]);
        }
        rts.insert(rts.end(),seg_rts.begin(),seg_rts.end());
    }
    return partition(M,rts);
}

static Piecewise<D2<SBasis> > sectionizeAtRoots(D2<SBasis> const &M){
    sectionizeAtCutsAndRoots(pwd2_to_d2pw(Piecewise<D2<SBasis> >(M)));
}

//=================================================================

static SBasis divide_by_sk(SBasis const &a, int k) {
    assert( k<a.size());
    if(k < 0) return shift(a,-k);
    SBasis c;
    c.insert(c.begin(), a.begin()+k, a.end());
    return c;
}


static SBasis divide_by_t0k(SBasis const &a, int k) {
    if(k < 0) {
        SBasis c = Linear(0,1);
        for (int i=2; i<=-k; i++){
            c*=c;
        }
        c*=a;
        return(c);
    }else{
        SBasis c = Linear(1,0);
        for (int i=2; i<=k; i++){
            c*=c;
        }
        c*=a;
        return(divide_by_sk(c,k));
    }
}

static SBasis divide_by_t1k(SBasis const &a, int k) {
    if(k < 0) {
        SBasis c = Linear(1,0);
        for (int i=2; i<=-k; i++){
            c*=c;
        }
        c*=a;
        return(c);
    }else{
        SBasis c = Linear(0,1);
        for (int i=2; i<=k; i++){
            c*=c;
        }
        c*=a;
        return(divide_by_sk(c,k));
    }
}

static D2<SBasis> RescaleForNonVanishingEnds(D2<SBasis> const &MM){
    D2<SBasis> M = MM;
    int val = min(valuation(M[0],ZERO),valuation(M[1],ZERO));

    //if M is the constant 0, what should we do? return empty, atm.
    if (val<0 || val>=max(M[0].size(),M[1].size())){
        return D2<SBasis>(SBasis(),SBasis());
    }

    if (val<M[0].size()) M[0] = divide_by_sk(M[0],val);
    if (val<M[1].size()) M[1] = divide_by_sk(M[1],val);
    while (fabs(M[0].at0())<ZERO && fabs(M[1].at0())<ZERO){
        M[0] = divide_by_t0k(M[0],1);
        M[1] = divide_by_t0k(M[1],1);
    }
    while (M[0].at1()==0 && M[1].at1()==0){
        M[0] = divide_by_t1k(M[0],1);
        M[1] = divide_by_t1k(M[1],1);
    }
    return M;
}

//=================================================================
static Piecewise<SBasis> 
curvature(D2<Piecewise<SBasis> > V, double tol=ZERO){
    Piecewise<SBasis> result;
    Piecewise<D2<SBasis> > VV = sectionizeAtCutsAndRoots(V);
    result.cuts.push_back(VV.cuts.front());
    for (int i=0; i<VV.size(); i++){
        Piecewise<SBasis> curv_seg;
        curv_seg = curvature(VV.segs[i],tol);
        curv_seg.setDomain(Interval(VV.cuts[i],VV.cuts[i+1]));
        result.concat(curv_seg);
    }
    return result;
}

static Piecewise<SBasis> 
curvature(Piecewise<D2<SBasis> > const &V, double tol=ZERO){
    Piecewise<SBasis> result;
    Piecewise<D2<SBasis> > VV = cutAtRoots(V);
    result.cuts.push_back(VV.cuts.front());
    for (int i=0; i<VV.size(); i++){
        Piecewise<SBasis> curv_seg;
        curv_seg = curvature(VV.segs[i],tol);
        curv_seg.setDomain(Interval(VV.cuts[i],VV.cuts[i+1]));
        result.concat(curv_seg);
    }
    return result;
}
//=================================================================

static Piecewise<D2<SBasis> >
MyUnitVector(D2<SBasis> const &V_in, double tol=ZERO, int order=3,
             double t0=0,double t1=1){
    //Unit vector(x,y) is (b,-a) where a and b are solutions of:
    //     ax+by=0 (eqn1)   and   a^2+b^2=1 (eqn2)
    D2<SBasis> V = RescaleForNonVanishingEnds(V_in);
    if (V[0].empty() && V[1].empty())
        return Piecewise<D2<SBasis> >(D2<SBasis>(Linear(1),SBasis()));
    SBasis x = V[0], y = V[1], a, b;
    SBasis r_eqn1, r_eqn2;

    Point v0 = unit_vector(V.at0());
    Point v1 = unit_vector(V.at1());
    a.push_back(Linear(-v0[1],-v1[1]));
    b.push_back(Linear( v0[0], v1[0]));

    r_eqn1 = -(a*x+b*y);
    r_eqn2 = Linear(1.)-(a*a+b*b);

    for (int k=1; k<=order; k++){
        double r0  = (k<r_eqn1.size())? r_eqn1.at(k).at0() : 0;
        double r1  = (k<r_eqn1.size())? r_eqn1.at(k).at1() : 0;
        double rr0 = (k<r_eqn2.size())? r_eqn2.at(k).at0() : 0;
        double rr1 = (k<r_eqn2.size())? r_eqn2.at(k).at1() : 0;
        double a0,a1,b0,b1;// coeffs in a[k] and b[k]

        //the equations to solve at this point are:
        // a0*x(0)+b0*y(0)=r0 & 2*a0*a(0)+2*b0*b(0)=rr0
        //and
        // a1*x(1)+b1*y(1)=r1 & 2*a1*a(1)+2*b1*b(1)=rr1
        a0 = r0/dot(v0,V(0))*v0[0]-rr0/2*v0[1];
        b0 = r0/dot(v0,V(0))*v0[1]+rr0/2*v0[0];
        a1 = r1/dot(v1,V(1))*v1[0]-rr1/2*v1[1];
        b1 = r1/dot(v1,V(1))*v1[1]+rr1/2*v1[0];

        a.push_back(Linear(a0,a1));        
        b.push_back(Linear(b0,b1));
        //TODO: explicit formulas -- make them 'incremental'.
        r_eqn1 = -(a*x+b*y);
        r_eqn2 = Linear(1)-(a*a+b*b);
    }
    
    //our candidat is:
    D2<SBasis> unitV;
    unitV[0] =  b;
    unitV[1] = -a;

    //is it good?
    double rel_tol = max(1.,max(V_in[0].tailError(0),V_in[1].tailError(0)))*tol;

    if (r_eqn1.tailError(order)>rel_tol || r_eqn2.tailError(order)>tol){
        //if not: subdivide and concat results.
        Piecewise<D2<SBasis> > unitV0, unitV1;
        unitV0 = MyUnitVector(compose(V,Linear(0,.5)),tol,order,t0,(t0+t1)/2);
        unitV1 = MyUnitVector(compose(V,Linear(.5,1)),tol,order,(t0+t1)/2,t1);
        unitV0.setDomain(Interval(0.,.5));
        unitV1.setDomain(Interval(.5,1.));
        unitV0.concat(unitV1);
        return(unitV0);
    }else{
        //if yes: return it as pw.
        Piecewise<D2<SBasis> > result;
        result=(Piecewise<D2<SBasis> >)unitV;
        return result;
    }
}

static Piecewise<D2<SBasis> >
MyUnitVector(Piecewise<D2<SBasis> > V, double tol=ZERO, int order=3){
    Piecewise<D2<SBasis> > result;
    Piecewise<D2<SBasis> > VV = cutAtRoots(V);
    result.cuts.push_back(VV.cuts.front());
    for (int i=0; i<VV.size(); i++){
        Piecewise<D2<SBasis> > unit_seg;
        unit_seg = MyUnitVector(VV.segs[i],tol, order);
        unit_seg.setDomain(Interval(VV.cuts[i],VV.cuts[i+1]));
        result.concat(unit_seg);   
    }
    return result;
}
     
#define SIZE 4
#define NB_SEGS 1

class SbCalculusToy: public Toy {

  void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
    
      Piecewise<D2<SBasis> >g;

      Piecewise<SBasis> f;
      g.cuts.push_back(0);
      for (int i=0; i<NB_SEGS; i++){
          D2<SBasis> seg;
          seg=handles_to_sbasis<SIZE-1>(handles.begin()+i*SIZE);
          g.push(seg,i+1);
      }

      cairo_set_line_width (cr, 0.5);
      cairo_set_source_rgba (cr, 0.9, 0., 0., 1);

//-- real->real functions------------------------------
//       SBasis x=Linear(-M_PI/2,2*M_PI);
//       f.cuts.push_back(0);
//       f.push(x,1);
//       plot(cr,cos(f),150);
//       plot(cr,sin(f),150);
//       plot(cr,Reciprocal(f,0.1),10);
//       plot(cr,my_sqrt(f),50);

//-- real->point functions------------------------------
      D2<SBasis> gseg=g.segs[0];
      double t0=0,t1=1;

      Piecewise<D2<SBasis> > dg;
      dg=derivative(g);

      Piecewise<D2<SBasis> >n = MyUnitVector(dg,.01,2);

//      Piecewise<SBasis> k = curvature(g,.1);
      n = rot90(n);
      n *= 150.;
      
      vector<double> cuts;
      vector<D2<SBasis> >uv = unit_vector(derivative(gseg),cuts,.01);
      D2<Piecewise<SBasis> >nn;
      for (int dim=0; dim<2; dim++){
          nn[dim].push_cut(0.);
          for (int i=0; i<cuts.size(); i++){
              nn[dim].push(uv[i][dim],cuts[i]);
          }
      }
      nn = rot90(nn);
      nn *= 148;

      D2<Piecewise<SBasis> > gsegpw;
      gsegpw[0]=Piecewise<SBasis>(gseg[0]);
      gsegpw[1]=Piecewise<SBasis>(gseg[1]);

      //cairo_d2_pw(cr, gsegpw);
      cairo_pw_d2(cr, g);
      cairo_set_source_rgba (cr, 0., 0.9, 0., .75);
      cairo_stroke(cr);

      cairo_set_source_rgba (cr, 0.9, 0., 0., .75);
      cairo_pw_d2(cr, g+n);
      //cairo_d2_pw(cr, g-n);
      for (int i=0; i<n.cuts.size(); i++){
          double t=n.cuts[i];
          draw_line_seg(cr, gseg(t), gseg(t)+n(t));
      }
      cairo_stroke(cr);

      cairo_set_source_rgba (cr, 0., 0., 0.9, .75);
      cairo_d2_pw(cr, gsegpw+nn);
      for (int i=0; i<nn[0].cuts.size(); i++){
          double t=nn[0].cuts[i];
          draw_line_seg(cr, gseg(t), gseg(t)+nn(t));
      }
      cairo_stroke(cr);

      *notify<<"unit_vector:\nBlue = old one, Red = new one.";
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
