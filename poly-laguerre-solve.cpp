#include "poly-laguerre-solve.h"
#include <iterator>

std::complex<double> Laguerre_internal_complex(Poly const & p,
                Poly const & pp,
                Poly const & ppp,
                double x0,
                double tol,
bool & quad_root) {
    std::complex<double> a = 2*tol;
    std::complex<double> xk = x0;
    double n = p.degree();
    quad_root = false;
    while(std::norm(a) > (tol*tol)) {
        //std::cout << "xk = " << xk << std::endl;
        std::complex<double> px = p(xk);
        if(std::norm(px) < tol*tol)
            return xk;
        std::complex<double> G = pp(xk) / px;
        std::complex<double> H = G*G - ppp(xk) / px;
        
        //std::cout << "G = " << G << "H = " << H;
        std::complex<double> radicand = (n - 1)*(n*H-G*G);
        //assert(radicand.real() > 0);
        if(radicand.real() < 0)
            quad_root = true;
        //std::cout << "radicand = " << radicand << std::endl;
        if(G.real() < 0) // here we try to maximise the denominator avoiding cancellation
            a = - sqrt(radicand);
        else
            a = sqrt(radicand);
        //std::cout << "a = " << a << std::endl;
        a = n / (a + G);
        //std::cout << "a = " << a << std::endl;
        xk -= a;
    }
    //std::cout << "xk = " << xk << std::endl;
    return xk;
}

double Laguerre_internal(Poly const & p,
                Poly const & pp,
                Poly const & ppp,
                double x0,
                double tol,
bool & quad_root) {
    double a = 2*tol;
    double xk = x0;
    double n = p.degree();
    quad_root = false;
    while(a*a > (tol*tol)) {
        //std::cout << "xk = " << xk << std::endl;
        double px = p(xk);
        if(px*px < tol*tol)
            return xk;
        double G = pp(xk) / px;
        double H = G*G - ppp(xk) / px;
        
        //std::cout << "G = " << G << "H = " << H;
        double radicand = (n - 1)*(n*H-G*G);
        assert(radicand > 0);
        //std::cout << "radicand = " << radicand << std::endl;
        if(G < 0) // here we try to maximise the denominator avoiding cancellation
            a = - sqrt(radicand);
        else
            a = sqrt(radicand);
        //std::cout << "a = " << a << std::endl;
        a = n / (a + G);
        //std::cout << "a = " << a << std::endl;
        xk -= a;
    }
    //std::cout << "xk = " << xk << std::endl;
    return xk;
}


std::vector<std::complex<double> > 
Laguerre(Poly p, const double tol) {
	std::vector<std::complex<double> > solutions;
	//std::cout << "p = " << p << " = ";
    while(p.size() > 1)
    {
        Poly pp = derivative(p),
            ppp = derivative(pp);
        double x0 = 0;
        double tol = 1e-15;
        bool quad_root = false;
        std::complex<double> sol = Laguerre_internal_complex(p, pp, ppp, x0, tol, quad_root);
        Poly dvs;
        if(quad_root) {
            dvs.push_back((sol*conj(sol)).real());
            dvs.push_back(-(sol + conj(sol)).real());
            dvs.push_back(1.0);
            //std::cout << "(" <<  dvs << ")";
	    solutions.push_back(sol);
	    solutions.push_back(conj(sol));
        } else {
            //std::cout << sol << std::endl;
            dvs.push_back(-sol.real());
            dvs.push_back(1.0);
	    solutions.push_back(sol);
            //std::cout << "(" <<  dvs << ")";
        }
        Poly r;
        p = divide(p, dvs, r);
        //std::cout << r << std::endl;
    }
    return solutions;
}

std::vector<double> 
Laguerre_real_interval(Poly const & ply, 
		       const double lo, const double hi,
		       const double tol) {
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
