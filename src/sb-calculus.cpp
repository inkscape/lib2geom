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
//TODO: define a truncated compose(sb,sb, order) and extend it to pw<sb>.
//TODO: in all these functions, compute 'order' according to 'tol'.

#include "sb-calculus.h"
#define ZERO 1e-3

using std::vector;
using namespace Geom;
using namespace std;

#include <stdio.h>
#include <math.h>

//-Sqrt---------------------------------------------------------------
Piecewise<SBasis> sqrtOnDomain(Interval range, double tol){
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

Piecewise<SBasis> sqrt(SBasis const &f, double tol, int order){
    Piecewise<SBasis> sqrt_fn=sqrtOnDomain(boundsFast(f), tol);
    Piecewise<SBasis> result=compose(sqrt_fn,f);
    //TODO: define a truncated compose()!
    for (int k=0; k<result.segs.size(); k++){result.segs[k].truncate(order);}
    return(result);
}
Piecewise<SBasis> sqrt(Piecewise<SBasis> const &f, double tol, int order){
    Piecewise<SBasis> sqrt_fn=sqrtOnDomain(boundsFast(f), tol);
    Piecewise<SBasis> result=compose(sqrt_fn,f);
    //TODO: define a truncated compose()!
    for (int k=0; k<result.segs.size(); k++){result.segs[k].truncate(order);}
    return(result);
}

//-sin/cos--------------------------------------------------------------
Piecewise<SBasis> cosOnDomain(Interval range){
    Piecewise<SBasis> cos_fn;
    SBasis cos0_90=cos(Linear(0,M_PI/2),3);
    int iMin=(int) floor(2*range.min()/M_PI);
    int iMax=(int) ceil(2*range.max()/M_PI);
    cos_fn.cuts.push_back(iMin*M_PI/2);
    for (int i=iMin; i<iMax; i++){
        double b=(i+1)*M_PI/2;
        //what does that stupid % operator do for <0 numbers ?!?
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

Piecewise<SBasis> cos(SBasis const &f){
    Piecewise<SBasis> cos_fn=cosOnDomain(boundsFast(f));
    return(compose(cos_fn,f));
}
Piecewise<SBasis> cos(Piecewise<SBasis> const &f){
    Piecewise<SBasis> cos_fn=cosOnDomain(boundsFast(f));
    return(compose(cos_fn,f));
}
Piecewise<SBasis> sin(SBasis const &f){
    SBasis g=f;
    g+=M_PI/2;
    Piecewise<SBasis> cos_fn=cosOnDomain(boundsFast(g));
    return(compose(cos_fn,g));
}
Piecewise<SBasis> sin(Piecewise<SBasis> const &f){
    Piecewise<SBasis> g=f;
    g+=M_PI/2;
    Piecewise<SBasis> cos_fn=cosOnDomain(boundsFast(g));
    return(compose(cos_fn,g));
}

//--1/x------------------------------------------------------------

Piecewise<SBasis> reciprocalOnDomain(Interval range, double tol){
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

Piecewise<SBasis> reciprocal(SBasis const &f, double tol, int order){
    Piecewise<SBasis> reciprocal_fn=reciprocalOnDomain(boundsFast(f), tol);
    Piecewise<SBasis> result=compose(reciprocal_fn,f);
    //TODO: define a truncated compose()!
    for (int k=0; k<result.segs.size(); k++){result.segs[k].truncate(order);}
    return(result);
}
Piecewise<SBasis> reciprocal(Piecewise<SBasis> const &f, double tol, int order){
    Piecewise<SBasis> reciprocal_fn=reciprocalOnDomain(boundsFast(f), tol);
    Piecewise<SBasis> result=compose(reciprocal_fn,f);
    //TODO: define a truncated compose()!
    for (int k=0; k<result.segs.size(); k++){result.segs[k].truncate(order);}
    return(result);
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
