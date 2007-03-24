#ifndef _SBASIS_GEOMETRIC
#define _SBASIS_GEOMETRIC
#include "d2.h"
#include "pw-sb.h"
#include <vector>

/** two-dimensional geometric operators.  
 * Copyright 2007, JFBarraud
 * Copyright 2007, njh
 * 
 * These operators are built on a more 'polynomially robust'
 * transformation to map a function that takes a [0,1] parameter to a
 * 2d vector into a function that takes the same [0,1] parameter to a
 * unit vector with the same direction.
 *
 * Rather that using (X/sqrt(X))(t) which involves two unstable
 * operations, sqrt and divide, this approach forms a curve directly
 * from the various tangent directions at each end (angular jet).  As
 * a result, the final path has a convergence behaviour derived from
 * that of the sin and cos series. -- njh
 */

namespace Geom{

std::vector<D2<SBasis> >
unit_vector(D2<SBasis> const vect, 
            std::vector<double> &cuts, 
            double tol=.1);

std::vector<D2<SBasis> > 
unit_vector(D2<SBasis> const vect, 
            double tol=.1);

std::vector<D2<SBasis> >
uniform_speed(D2<SBasis> const M, 
              double tol=.1);

double
arc_length(D2<SBasis> const M, 
           double tol=.1);


Piecewise<SBasis>
arc_length_sb(D2<SBasis> const M, 
              double tol=.1);

Piecewise<SBasis>
curvature(D2<SBasis> const M, 
          double tol=.1);

};

#endif

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

 	  	 
