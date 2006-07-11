#include "solve-sbasis.h"
#include "point-fns.h"

#define SGN(a)      (((a)<0) ? -1 : 0)

static double 
SquaredLength(const Geom::Point a) {
    return dot(a, a);
}


/*
 *  Forward declarations
 */
static Geom::Point 
Bezier(Geom::Point *V,
       int degree,
       double t,
       Geom::Point *Left,
       Geom::Point *Right);

int
crossing_count(Geom::Point *V, int degree);
static int 
control_poly_flat_enough(Geom::Point *V, int degree);
static double
compute_x_intercept(Geom::Point *V, int degree);

int	MAXDEPTH = 64;	/*  Maximum depth for recursion */

#define	EPSILON	(ldexp(1.0,-MAXDEPTH-1)) /*Flatness control value */

/*
 *  find_bezier_roots : Given a 5th-degree equation in Bernstein-Bezier form, find all of the roots
 *in the interval [0, 1].  Return the number of roots found.
 */
int
find_bezier_roots(
    Geom::Point *w, /* The control points  */
    int degree,	/* The degree of the polynomial */
    double *t,	/* RETURN candidate t-values */
    int depth)	/* The depth of the recursion */
{  
    int i;
    Geom::Point Left[degree+1],	/* New left and right  */
        Right[degree+1];	/* control polygons  */
    int left_count,		/* Solution count from  */
        right_count;		/* children  */
    double left_t[degree+1],	/* Solutions from kids  */
        right_t[degree+1];

    switch (crossing_count(w, degree)) {
    case 0: 	/* No solutions here	*/
        return 0;
	
    case 1:
 	/* Unique solution	*/
        /* Stop recursion when the tree is deep enough	*/
        /* if deep enough, return 1 solution at midpoint  */
        if (depth >= MAXDEPTH) {
            t[0] = (w[0][Geom::X] + w[degree][Geom::X]) / 2.0;
            return 1;
        }
        if (control_poly_flat_enough(w, degree)) {
            t[0] = compute_x_intercept(w, degree);
            return 1;
        }
        break;
    }

    /* Otherwise, solve recursively after subdividing control polygon  */
    Bezier(w, degree, 0.5, Left, Right);
    left_count  = find_bezier_roots(Left,  degree, left_t, depth+1);
    right_count = find_bezier_roots(Right, degree, right_t, depth+1);


    /* Gather solutions together	*/
    for (i = 0; i < left_count; i++) {
        t[i] = left_t[i];
    }
    for (i = 0; i < right_count; i++) {
        t[i+left_count] = right_t[i];
    }

    /* Send back total number of solutions	*/
    return (left_count+right_count);
}


/*
 * crossing_count:
 *  Count the number of times a Bezier control polygon 
 *  crosses the 0-axis. This number is >= the number of roots.
 *
 */
int
crossing_count(Geom::Point *V,	/*  Control pts of Bezier curve	*/
	       int degree)	/*  Degree of Bezier curve 	*/
{
    int 	n_crossings = 0;	/*  Number of zero-crossings */
    int		old_sign;		/*  Sign of coefficients */
    
    old_sign = SGN(V[0][Geom::Y]);
    for (int i = 1; i <= degree; i++) {
        int sign = SGN(V[i][Geom::Y]);
        if (sign != old_sign)
            n_crossings++;
        old_sign = sign;
    }
    return n_crossings;
}



/*
 *  control_poly_flat_enough :
 *	Check if the control polygon of a Bezier curve is flat enough
 *	for recursive subdivision to bottom out.
 *
 */
static int 
control_poly_flat_enough(Geom::Point *V, /* Control points	*/
			 int degree)	/* Degree of polynomial	*/
{
    /* Find the perpendicular distance from each interior control point to line connecting V[0] and
     * V[degree] */
    double *distance = new double[degree + 1]; /* Distances from pts to line */

    /* Derive the implicit equation for line connecting first */
    /*  and last control points */
    const double a = V[0][Geom::Y] - V[degree][Geom::Y];
    const double b = V[degree][Geom::X] - V[0][Geom::X];
    const double c = V[0][Geom::X] * V[degree][Geom::Y] - V[degree][Geom::X] * V[0][Geom::Y];

    const double abSquared = (a * a) + (b * b);

    for (unsigned i = 1; i < degree; i++) {
        /* Compute distance from each of the points to that line */
        distance[i] = a * V[i][Geom::X] + b * V[i][Geom::Y] + c;
        if (distance[i] > 0.0) {
            distance[i] = (distance[i] * distance[i]) / abSquared;
        }
        if (distance[i] < 0.0) {
            distance[i] = -(distance[i] * distance[i]) / abSquared;
        }
    }


    /* Find the largest distance	*/
    double max_distance_above = 0.0;
    double max_distance_below = 0.0;
    for (unsigned i = 1; i < degree; i++) {
        if (distance[i] < 0.0) {
            max_distance_below = MIN(max_distance_below, distance[i]);
        };
        if (distance[i] > 0.0) {
            max_distance_above = MAX(max_distance_above, distance[i]);
        }
    }
    delete[] distance;

    /*  Implicit equation for zero line */
    const double a1 = 0.0;
    const double b1 = 1.0;
    const double c1 = 0.0;

    /*  Implicit equation for "above" line */
    double a2 = a;
    double b2 = b;
    double c2 = c + max_distance_above;

    double det = a1 * b2 - a2 * b1;
	
    const double intercept_1 = (b1 * c2 - b2 * c1) / det;

    /*  Implicit equation for "below" line */
    a2 = a;
    b2 = b;
    c2 = c + max_distance_below;
	
    det = a1 * b2 - a2 * b1;
	
    const double intercept_2 = (b1 * c2 - b2 * c1) / det;

    /* Compute intercepts of bounding box	*/
    const double left_intercept = std::min(intercept_1, intercept_2);
    const double right_intercept = std::max(intercept_1, intercept_2);

    const double error = 0.5 * (right_intercept - left_intercept);
    
    if (error < EPSILON)
        return 1;
    
    return 0;
}



/*
 *  compute_x_intercept :
 *	Compute intersection of chord from first control point to last
 *  	with 0-axis.
 * 
 */
static double
compute_x_intercept(Geom::Point *V, /*  Control points	*/
		    int degree) /*  Degree of curve	*/
{
    const Geom::Point A = V[degree] - V[0];

    return (A[Geom::X]*V[0][Geom::Y] - A[Geom::Y]*V[0][Geom::X]) / -A[Geom::Y];
}


/*
 *  Bezier : 
 *	Evaluate a Bezier curve at a particular parameter value
 *      Fill in control points for resulting sub-curves if "Left" and
 *	"Right" are non-null.
 * 
 */
static Geom::Point 
Bezier(Geom::Point *V, /* Control pts	*/
       int degree,	/* Degree of bezier curve */
       double t,	/* Parameter value */
       Geom::Point *Left,	/* RETURN left half ctl pts */
       Geom::Point *Right)	/* RETURN right half ctl pts */
{
    Geom::Point Vtemp[degree+1][degree+1];

    /* Copy control points	*/
    for (unsigned j =0; j <= degree; j++)
        Vtemp[0][j] = V[j];

    /* Triangle computation	*/
    for (unsigned i = 1; i <= degree; i++) {	
        for (unsigned j =0 ; j <= degree - i; j++) {
            Vtemp[i][j] = (1.0 - t) * Vtemp[i-1][j] 
                + t * Vtemp[i-1][j+1];
        }
    }
    
    if (Left != NULL)
        for (unsigned j = 0; j <= degree; j++)
            Left[j]  = Vtemp[j][0];
    if (Right != NULL)
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

