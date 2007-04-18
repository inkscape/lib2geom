/** root finding for sbasis functions.
 * Copyright 2006 N Hurst
 * Copyright 2007 JF Barraud
 *
 * It is more efficient to find roots of f(t) = c_0, c_1, ... all at once, rather than iterating.
 *
 * Todo/think about:
 *  multi-roots using bernstein method, one approach would be:
    sort c
    take median and find roots of that
    whenever a segment lies entirely on one side of the median, 
    find the median of the half and recurse.

    in essence we are implementing quicksort on a continuous function

 *  the gsl poly roots finder is faster than bernstein too, but we don't use it for 3 reasons:

 a) it requires convertion to poly, which is numerically unstable

 b) it requires gsl (which is currently not a dependency, and would bring in a whole slew of unrelated stuff)

 c) it finds all roots, even complex ones.  We don't want to accidently treat a nearly real root as a real root

From memory gsl poly roots was about 10 times faster than bernstein in the case where all the roots
are in [0,1] for polys of order 5.  I spent some time working out whether eigenvalue root finding
could be done directly in sbasis space, but the maths was too hard for me. -- njh

jfbarraud: eigenvalue root finding could be done directly in sbasis space ?

njh: I don't know, I think it should.  You would make a matrix whose characteristic polynomial was
correct, but do it by putting the sbasis terms in the right spots in the matrix.  normal eigenvalue
root finding makes a matrix that is a diagonal + a row along the top.  This matrix has the property
that its characteristic poly is just the poly whose coefficients are along the top row.

Now an sbasis function is a linear combination of the poly coeffs.  So it seems to me that you
should be able to put the sbasis coeffs directly into a matrix in the right spots so that the
characteristic poly is the sbasis.  You'll still have problems b) and c).

We might be able to lift an eigenvalue solver and include that directly into 2geom.  Eigenvalues
also allow you to find intersections of multiple curves but require solving n*m x n*m matrices.

 **/

#include <cmath>
#include <map>

#include "s-basis.h"
#include "sbasis-to-bezier.h"
#include "solver.h"

namespace Geom{

Interval SBasis::boundsFast(int order) const {
    int imax=size()-1;
    double lo = 0.0, hi = 0.0;

    for(int i = imax; i >=order; i--) {
      double a=(*this)[i][0];
      double b=(*this)[i][1];
      double t;

      if (hi > 0){ t=((b-a)+hi)/2/hi;}
      if (hi<=0||t<0||t>1){
	hi=std::max(a,b);
      }else{
	hi=a*(1-t)+b*t+hi*t*(1-t);	  
      }

      if (lo<0){t=((b-a)+lo)/2/lo;}
      if (lo>=0||t<0||t>1){
	lo=std::min(a,b);
      }else{
	lo=a*(1-t)+b*t+lo*t*(1-t);	  
      }
    }
    if (order>0){
        lo*=pow(.25,order);
        hi*=pow(.25,order);
    }
    return Interval(lo, hi);
}

Interval SBasis::boundsLocal(double t0, double t1, int order) const {
    int imax=size()-1;
    double lo = 0.0, hi = 0.0;
    
    for(int i = imax; i >=order; i--) {
        double a=(*this)[i][0];
        double b=(*this)[i][1];
        double t;
        if (hi>0){t=((b-a)+hi)/2/hi;}
        if (hi<=0||t<t0||t>t1){
            hi=std::max(a*(1-t0)+b*t0+hi*t0*(1-t0),a*(1-t1)+b*t1+hi*t1*(1-t1));
        }else{
            hi=a*(1-t)+b*t+hi*t*(1-t);	  
        }
        if (lo<0){t=((b-a)+lo)/2/lo;}
        if (lo>=0||t<t0||t>t1){
            lo=std::min(a*(1-t0)+b*t0+lo*t0*(1-t0),a*(1-t1)+b*t1+lo*t1*(1-t1));
        }else{
            lo=a*(1-t)+b*t+lo*t*(1-t);	  
        }
    }
    if (order>0){
        lo*=pow(.25,order);
        hi*=pow(.25,order);
    }
    return Interval(lo, hi);
}

//-- multi_roots ------------------------------------
// goal: solve f(t)=c for several c at once.
/* algo: -compute f at both ends of the given segment [a,b].
         -compute bounds m<df(t)<M for df on the segment.
         let c and C be the levels below and above f(a):
         going from f(a) down to c with slope m takes at least time (f(a)-c)/m
         going from f(a)  up  to C with slope M takes at least time (C-f(a))/M
         From this we conclude there are no roots before a'=a+min((f(a)-c)/m,(C-f(a))/M).
         Do the same for b: compute some b' such that there are no roots in (b',b].
         -if [a',b'] is not empty, repeat the process with [a',(a'+b')/2] and [(a'+b')/2,b']. 
  unfortunately, extra care is needed about rounding errors, and also to avoid the repetition of roots,
  making things tricky and unpleasant...
*/
//TODO: Make sure the code is "rounding-errors proof" and take care about repetition of roots!


static int upper_level(vector<double> const &levels,double x,double tol=0.){
    return(upper_bound(levels.begin(),levels.end(),x-tol)-levels.begin());
}

static void multi_roots_internal(SBasis const &f,
				 SBasis const &df,
				 std::vector<double> const &levels,
				 std::vector<std::vector<double> > &roots,
				 double tol,
				 double a,
				 double fa,
				 double b,
				 double fb){
    
    if (f.size()==0){
        int idx;
        idx=upper_level(levels,0,tol);
        if (idx<levels.size()&&fabs(levels[idx])<=tol){
            roots[idx].push_back(a);
            roots[idx].push_back(b);
        }
        return;
    }
////usefull? 
//     if (f.size()==1){
//         int idxa=upper_level(levels,fa);
//         int idxb=upper_level(levels,fb);
//         if (fa==fb){
//             if (fa==levels[idxa]){
//                 roots[a]=idxa;
//                 roots[b]=idxa;
//             }
//             return;
//         }
//         int idx_min=std::min(idxa,idxb);
//         int idx_max=std::max(idxa,idxb);
//         if (idx_max==levels.size()) idx_max-=1;
//         for(int i=idx_min;i<=idx_max; i++){
//             double t=a+(b-a)*(levels[i]-fa)/(fb-fa);
//             if(a<t&&t<b) roots[t]=i;
//         }
//         return;
//     }
    if ((b-a)<tol){
        //TODO: use different tol for t and f ?
        int idx=std::min(upper_level(levels,fa,tol),upper_level(levels,fb,tol));
        if (idx==levels.size()) idx-=1;
        double c=levels[idx];
        if((fa-c)*(fb-c)<=0||fabs(fa-c)<tol||fabs(fb-c)<tol){
            roots[idx].push_back((a+b)/2);
        }
        return;
    }
    
    int idxa=upper_level(levels,fa,tol);
    int idxb=upper_level(levels,fb,tol);
    
    Interval bs = df.boundsLocal(a,b);

    //first times when a level (higher or lower) can be reached from a or b.
    double ta_hi,tb_hi,ta_lo,tb_lo;
    ta_hi=ta_lo=b+1;//default values => no root there.
    tb_hi=tb_lo=a-1;//default values => no root there.

    if (fabs(fa-levels[idxa])<tol){//a can be considered a root.
        //ta_hi=ta_lo=a;
        roots[idxa].push_back(a);
        ta_hi=ta_lo=a+tol;
    }else{
        if (bs.max()>0 && idxa<levels.size())
            ta_hi=a+(levels[idxa  ]-fa)/bs.max();
        if (bs.min()<0 && idxa>0)
            ta_lo=a+(levels[idxa-1]-fa)/bs.min();
    }
    if (fabs(fb-levels[idxb])<tol){//b can be considered a root.
        //tb_hi=tb_lo=b;
        roots[idxb].push_back(b);
        tb_hi=tb_lo=b-tol;
    }else{
        if (bs.min()<0 && idxb<levels.size())
            tb_hi=b+(levels[idxb  ]-fb)/bs.min();
        if (bs.max()>0 && idxb>0)
            tb_lo=b+(levels[idxb-1]-fb)/bs.max();
    }
    
    double t0,t1;
    t0=std::min(ta_hi,ta_lo);    
    t1=std::max(tb_hi,tb_lo);
    //hum, rounding errors frighten me! so I add this +tol...
    if (t0>t1+tol) return;//no root here.
    if (fabs(t1-t0)<tol){
        multi_roots_internal(f,df,levels,roots,tol,t0,f(t0),t1,f(t1));
    }else{
        double t,t_left,t_right,ft,ft_left,ft_right;
        t_left =t_right =t =(t0+t1)/2;
        ft_left=ft_right=ft=f(t);
        int idx=upper_level(levels,ft,tol);
        if (fabs(ft-levels[idx])<tol){//t can be considered a root.
            roots[idx].push_back(t);
            //we do not want to count it twice (from the left and from the right)
            t_left =t-tol/2;
            t_right=t+tol/2;
            ft_left =f(t_left);
            ft_right=f(t_right);
        }
        multi_roots_internal(f,df,levels,roots,tol,t0     ,f(t0)   ,t_left,ft_left);
        multi_roots_internal(f,df,levels,roots,tol,t_right,ft_right,t1    ,f(t1)  );
    }
}

std::vector<std::vector<double> > multi_roots(SBasis const &f,
                                      std::vector<double> const &levels,
                                      double tol,
                                      double a,
                                      double b){
    std::vector<std::vector<double> > roots;
    for(int i=0;i<levels.size();i++){
        roots.push_back(std::vector<double>());
    }
    SBasis df=derivative(f);
    multi_roots_internal(f,df,levels,roots,tol,a,f(a),b,f(b));  
    return(roots);
}
//-------------------------------------

#if 0
double Laguerre_internal(SBasis const & p,
                         double x0,
                         double tol,
                         bool & quad_root) {
    double a = 2*tol;
    double xk = x0;
    double n = p.size();
    quad_root = false;
    while(a > tol) {
        //std::cout << "xk = " << xk << std::endl;
        Linear b = p.back();
        Linear d(0), f(0);
        double err = fabs(b);
        double abx = fabs(xk);
        for(int j = p.size()-2; j >= 0; j--) {
            f = xk*f + d;
            d = xk*d + b;
            b = xk*b + p[j];
            err = fabs(b) + abx*err;
        }
        
        err *= 1e-7; // magic epsilon for convergence, should be computed from tol
        
        double px = b;
        if(fabs(b) < err)
            return xk;
        //if(std::norm(px) < tol*tol)
        //    return xk;
        double G = d / px;
        double H = G*G - f / px;
        
        //std::cout << "G = " << G << "H = " << H;
        double radicand = (n - 1)*(n*H-G*G);
        //assert(radicand.real() > 0);
        if(radicand < 0)
            quad_root = true;
        //std::cout << "radicand = " << radicand << std::endl;
        if(G.real() < 0) // here we try to maximise the denominator avoiding cancellation
            a = - std::sqrt(radicand);
        else
            a = std::sqrt(radicand);
        //std::cout << "a = " << a << std::endl;
        a = n / (a + G);
        //std::cout << "a = " << a << std::endl;
        xk -= a;
    }
    //std::cout << "xk = " << xk << std::endl;
    return xk;
}
#endif

void subdiv_sbasis(SBasis const & s,
                   std::vector<double> & roots, 
                   double left, double right) {
    Interval bs = s.boundsFast();
    if(bs.min() > 0 || bs.max() < 0)
        return; // no roots here
    if(s.tailError(1) < 1e-7) {
        double t = s[0][0] / (s[0][0] - s[0][1]);
        roots.push_back(left*(1-t) + t*right);
        return;
    }
    double middle = (left + right)/2;
    subdiv_sbasis(compose(s, Linear(0, 0.5)), roots, left, middle);
    subdiv_sbasis(compose(s, Linear(0.5, 1.)), roots, middle, right);
}

// It is faster to use the bernstein root finder for small degree polynomials (<100?.

std::vector<double> roots(SBasis const & s) {
    if(s.size() == 0) return std::vector<double>();
    std::vector<double> b = sbasis_to_bezier(s), r;
    
    find_bernstein_roots(&b[0], b.size()-1, r, 0, 0., 1.);
    return r;
}

};

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
