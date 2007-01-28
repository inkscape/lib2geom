#include <cmath>

#include "s-basis.h"
#include "sbasis-to-bezier.h"
#include "solver.h"

namespace Geom{

// void bounds(SBasis const & s,
//             double &lo, double &hi) {
//     double ss = 0.25;
//     double mid = s(0.5);
//     lo = hi = mid;
//     for(unsigned i = 1; i < s.size(); i++) {
//         for(unsigned dim = 0; dim < 2; dim++) {
//             double b = s[i][dim]*ss;
//             if(b < 0)
//                 lo += b;
//             if(b > 0)
//                 hi += b;
//         }
//         ss *= 0.25;
//     }
//     lo = std::min(std::min(lo, s[0][1]),s[0][0]);
//     hi = std::max(std::max(hi, s[0][0]), s[0][1]);
// }
void bounds(SBasis const & s,
	    double &lo, double &hi, int order) {
    int imax=s.size()-1;
    lo=0;
    hi=0;

    for(int i = imax; i >=order; i--) {
      double a=s[i][0];
      double b=s[i][1];
      double t;

      if (hi>0){t=((b-a)+hi)/2/hi;}
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
}

void slow_bounds(SBasis const & f_in,
			double &m, 
			double &M,
			int order,
			double tol
			) {

  assert (tol>0);
  order=std::max(0,order);
  SBasis f;
  for (int i=order;i<f_in.size();i++){
    f.push_back(f_in[i]);
  }

  double Mf,mf,Mdf,mdf,Md2f,md2f,t,h,err;
  double step,step_min;
  SBasis df=derivative(f);
  SBasis d2f=derivative(df);
  bounds(df,mdf,Mdf);
  bounds(d2f,md2f,Md2f);
  if (f.size()<=1||mdf>=0||Mdf<=0){
    bounds(f,m,M);
  }else{
    bounds(f,mf,Mf);
    err=tol*std::max(1.,Mf-mf);//Fix me: 
    //1)Mf-mf should not be 0...
    //2)tolerance relative to inaccurate bounds (but using
    // the current value of M and m can be speed consuming).
    double err_M=0,err_m=0;
    step_min=err/std::max(Mdf,-mdf);
    M=std::max(f[0][0],f[0][1]);
    m=std::min(f[0][0],f[0][1]);
    for (t=0;t<=1;){
      double ft=f(t);
      double dft=df(t);
      M=std::max(M,ft);
      m=std::min(m,ft);
      if (M<ft){
	M=ft;
	err_M=dft*step_min;
      }
      if (m>ft){
	m=ft;
	err_m=-dft*step_min;
      }
      if (dft>0){
	step=std::max(-dft/md2f,step_min);
      }else if (dft<0){
	step=std::max(-dft/Md2f,step_min);
      }else{
	step=step_min;
      }
      step=std::max(step,std::min((M-ft)/Mdf,(m-ft)/mdf));
      t+=step;
    }
    M+=err_M;
    m-=err_m;
    M*=pow(.25,order);
    m*=pow(.25,order);
  }
}


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
        BezOrd b = p.back();
        BezOrd d(0), f(0);
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
    double lo, hi;
    bounds(s, lo, hi);
    if(lo > 0 || hi < 0)
        return; // no roots here
    if(s.tail_error(1) < 1e-7) {
        double t = s[0][0] / (s[0][0] - s[0][1]);
        roots.push_back(left*(1-t) + t*right);
        return;
    }
    double middle = (left + right)/2;
    subdiv_sbasis(compose(s, BezOrd(0, 0.5)), roots, left, middle);
    subdiv_sbasis(compose(s, BezOrd(0.5, 1.)), roots, middle, right);
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
