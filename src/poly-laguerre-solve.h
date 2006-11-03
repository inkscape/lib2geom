#include "poly.h"
#include <complex>

std::vector<std::complex<double> > 
Laguerre(Poly ply, const double tol=1e-10);

std::vector<double> 
Laguerre_real_interval(Poly  ply, 
		       const double lo, const double hi,
		       const double tol=1e-10);
