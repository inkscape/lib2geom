#include "solver.h"
#include "point-fns.h"
#include <algorithm>

/*** Find the zeros of the bernstein function.  The code subdivides until it is happy with the
 * linearity of the function.  This requires an O(degree^2) subdivision for each step, even when
 * there is only one solution.
 */

#define SGN(a)      (((a)<0) ? -1 : 1)

/*
 *  Forward declarations
 */
static double 
Bernstein(double const *V,
          unsigned degree,
          double t,
          double *Left,
          double *Right);

unsigned
crossing_count(double const *V, unsigned degree);
static unsigned 
control_poly_flat_enough(double const *V, unsigned degree,
			 double left_t, double right_t);

const int MAXDEPTH = 64;	/*  Maximum depth for recursion */

const double EPSILON = ldexp(1.0,-MAXDEPTH-1); /*Flatness control value */

/*
 *  find_bernstein_roots : Given an equation in Bernstein-Bernstein form, find all 
 *    of the roots in the interval [0, 1].  Return the number of roots found.
 */
void
find_bernstein_roots(double const *w, /* The control points  */
                     unsigned degree,	/* The degree of the polynomial */
                     std::vector<double> &solutions, /* RETURN candidate t-values */
                     unsigned depth,	/* The depth of the recursion */
                     double left_t, double right_t)
{  
    unsigned 	n_crossings = 0;	/*  Number of zero-crossings */
    
    int old_sign = SGN(w[0]);
    for (int i = 1; i <= degree; i++) {
        int sign = SGN(w[i]);
        if (sign != old_sign)
            n_crossings++;
        old_sign = sign;
    }
    switch (n_crossings) {
    case 0: 	/* No solutions here	*/
        return;
	
    case 1:
 	/* Unique solution	*/
        /* Stop recursion when the tree is deep enough	*/
        /* if deep enough, return 1 solution at midpoint  */
        if (depth >= MAXDEPTH) {
            solutions.push_back((left_t + right_t) / 2.0);
            return;
        }
        
        // I thought secant method would be faster here, but it'aint. -- njh

        if (control_poly_flat_enough(w, degree, left_t, right_t)) {
            const double ts = ((right_t - left_t)*w[0]) / (w[degree] - w[0]);
            solutions.push_back(left_t - ts);
            return;
        }
        break;
    }

    /* Otherwise, solve recursively after subdividing control polygon  */
    double Left[degree+1],	/* New left and right  */
        Right[degree+1];	/* control polygons  */
    old_sign = SGN(w[0]);

    for (int i = 1; i <= degree; i++) {
        int sign = SGN(w[i]);
        if (sign != old_sign) {
            double first_split = (double(i))/(degree+1);
            Bernstein(w, degree, first_split, Left, Right);
            double mid_t = first_split*(left_t + right_t);
            find_bernstein_roots(Left,  degree, solutions, depth+1, 
                                 left_t, mid_t);
            find_bernstein_roots(Right, degree, solutions, depth+1, 
                                 mid_t, right_t);
            return;
        }
    }
}


/*
 * crossing_count:
 *  Count the number of times a Bernstein control polygon 
 *  crosses the 0-axis. This number is >= the number of roots.
 *
 */
unsigned
crossing_count(double const *V,	/*  Control pts of Bernstein curve	*/
	       unsigned degree)	/*  Degree of Bernstein curve 	*/
{
    unsigned 	n_crossings = 0;	/*  Number of zero-crossings */
    
    int old_sign = SGN(V[0]);
    for (int i = 1; i <= degree; i++) {
        int sign = SGN(V[i]);
        if (sign != old_sign)
            n_crossings++;
        old_sign = sign;
    }
    return n_crossings;
}



/*
 *  control_poly_flat_enough :
 *	Check if the control polygon of a Bernstein curve is flat enough
 *	for recursive subdivision to bottom out.
 *
 */
static unsigned 
control_poly_flat_enough(double const *V, /* Control points	*/
			 unsigned degree,
			 double left_t, double right_t)	/* Degree of polynomial	*/
{
    /* Find the perpendicular distance from each interior control point to line connecting V[0] and
     * V[degree] */

    /* Derive the implicit equation for line connecting first */
    /*  and last control points */
    const double a = V[0] - V[degree];
    const double b = right_t - left_t;
    const double c = left_t * V[degree] - right_t * V[0];

    const double abSquared = (a * a) + (b * b);

    double distance[degree]; /* Distances from pts to line */
    for (unsigned i = 1; i < degree; i++) {
        /* Compute distance from each of the points to that line */
        double & dist(distance[i-1]);
        const double d = a * ((double(i) / degree)*b + left_t) + b * V[i] + c;
        dist = d*d / abSquared;
        if (d < 0.0)
            dist = -dist;
    }


    // Find the largest distance
    double max_distance_above = 0.0;
    double max_distance_below = 0.0;
    for (unsigned i = 0; i < degree-1; i++) {
        const double d = distance[i];
        if (d < 0.0)
            max_distance_below = MIN(max_distance_below, d);
        if (d > 0.0)
            max_distance_above = MAX(max_distance_above, d);
    }
    
    const double intercept_1 = (c + max_distance_above) / -a;
    const double intercept_2 = (c + max_distance_below) / -a;

    /* Compute bounding interval*/
    const double left_intercept = std::min(intercept_1, intercept_2);
    const double right_intercept = std::max(intercept_1, intercept_2);

    const double error = (right_intercept - left_intercept);
    
    if (error < EPSILON*2)
        return 1;
    
    return 0;
}



/*
 *  Bernstein : 
 *	Evaluate a Bernstein function at a particular parameter value
 *      Fill in control points for resulting sub-curves.
 * 
 */
static double 
Bernstein(double const *V, /* Control pts	*/
          unsigned degree,	/* Degree of bernstein curve */
          double t,	/* Parameter value */
          double *Left,	/* RETURN left half ctl pts */
          double *Right)	/* RETURN right half ctl pts */
{
    double Vtemp[degree+1][degree+1];

    /* Copy control points	*/
    std::copy(V, V+degree+1, Vtemp[0]);

    /* Triangle computation	*/
    for (unsigned i = 1; i <= degree; i++) {	
        for (unsigned j = 0; j <= degree - i; j++) {
            Vtemp[i][j] = (1-t)*Vtemp[i-1][j] + t*Vtemp[i-1][j+1];
        }
    }
    
    for (unsigned j = 0; j <= degree; j++)
        Left[j]  = Vtemp[j][0];
    for (unsigned j = 0; j <= degree; j++)
        Right[j] = Vtemp[degree-j][j];

    return (Vtemp[degree][0]);
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(substatement-open . 0))
  indent-tabs-mode:nil
  c-brace-offset:0
  fill-column:99
  End:
  vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/

