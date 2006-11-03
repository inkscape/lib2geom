#include <math.h>
#include <stdlib.h>
#include "path-intersect.h"
namespace Geom{
/* The value of 1.0 / (1L<<14) is enough for most applications */
const double INV_EPS = (1L<<14);
    
/*
 * Split the curve at the midpoint, returning an array with the two parts
 * Temporary storage is minimized by using part of the storage for the result
 * to hold an intermediate value until it is no longer needed.
 */
#define left r[0]
#define right r[1]
Bezier *Bezier::Split() {
    Bezier *r = new Bezier[2];
    left.p[0] = p[0];
    right.p[3] = p[3];
    left.p[1] = Lerp(0.5, p[0], p[1]);
    right.p[2] = Lerp(0.5, p[2], p[3]);
    right.p[1] = Lerp(0.5, p[1], p[2]); // temporary holding spot
    left.p[2] = Lerp(0.5, left.p[1], right.p[1]);
    right.p[1] = Lerp(0.5, right.p[1], right.p[2]); // Real value this time
    left.p[3] = right.p[0] = Lerp(0.5, left.p[2], right.p[1]);
    return r;
}
#undef left
#undef right

    
/*
 * Test the bounding boxes of two Bezier curves for interference.
 * Several observations:
 *	First, it is cheaper to compute the bounding box of the second curve
 *	and test its bounding box for interference than to use a more direct
 *	approach of comparing all control points of the second curve with 
 *	the various edges of the bounding box of the first curve to test
 * 	for interference.
 *	Second, after a few subdivisions it is highly probable that two corners
 *	of the bounding box of a given Bezier curve are the first and last 
 *	control point.  Once this happens once, it happens for all subsequent
 *	subcurves.  It might be worth putting in a test and then short-circuit
 *	code for further subdivision levels.
 *	Third, in the final comparison (the interference test) the comparisons
 *	should both permit equality.  We want to find intersections even if they
 *	occur at the ends of segments.
 *	Finally, there are tighter bounding boxes that can be derived. It isn't
 *	clear whether the higher probability of rejection (and hence fewer
 *	subdivisions and tests) is worth the extra work.
 */

// And then he discovered for loops...
int IntersectBB( Bezier a, Bezier b )
{
    // Compute bounding box for a
    double minax, maxax, minay, maxay;
    if( a.p[0][X] > a.p[3][X] )	 // These are the most likely to be extremal
	minax = a.p[3][X], maxax = a.p[0][X];
    else
	minax = a.p[0][X], maxax = a.p[3][X];
    if( a.p[2][X] < minax )
	minax = a.p[2][X];
    else if( a.p[2][X] > maxax )
	maxax = a.p[2][X];
    if( a.p[1][X] < minax )
	minax = a.p[1][X];
    else if( a.p[1][X] > maxax )
	maxax = a.p[1][X];
    if( a.p[0][Y] > a.p[3][Y] ) 		
	minay = a.p[3][Y], maxay = a.p[0][Y];
    else
	minay = a.p[0][Y], maxay = a.p[3][Y];
    if( a.p[2][Y] < minay )
	minay = a.p[2][Y];
    else if( a.p[2][Y] > maxay )
	maxay = a.p[2][Y];
    if( a.p[1][Y] < minay )
	minay = a.p[1][Y];
    else if( a.p[1][Y] > maxay )
	maxay = a.p[1][Y];
    // Compute bounding box for b
    double minbx, maxbx, minby, maxby;
    if( b.p[0][X] > b.p[3][X] ) 		
	minbx = b.p[3][X], maxbx = b.p[0][X];
    else
	minbx = b.p[0][X], maxbx = b.p[3][X];
    if( b.p[2][X] < minbx )
	minbx = b.p[2][X];
    else if( b.p[2][X] > maxbx )
	maxbx = b.p[2][X];
    if( b.p[1][X] < minbx )
	minbx = b.p[1][X];
    else if( b.p[1][X] > maxbx )
	maxbx = b.p[1][X];
    if( b.p[0][Y] > b.p[3][Y] ) 		
	minby = b.p[3][Y], maxby = b.p[0][Y];
    else
	minby = b.p[0][Y], maxby = b.p[3][Y];
    if( b.p[2][Y] < minby )
	minby = b.p[2][Y];
    else if( b.p[2][Y] > maxby )
	maxby = b.p[2][Y];
    if( b.p[1][Y] < minby )
	minby = b.p[1][Y];
    else if( b.p[1][Y] > maxby )
	maxby = b.p[1][Y];
    // Test bounding box of b against bounding box of a
    if( ( minax > maxbx ) || ( minay > maxby )  // Not >= : need boundary case
	|| ( minbx > maxax ) || ( minby > maxay ) )
	return 0; // they don't intersect
    else
	return 1; // they intersect
}
	
/* 
 * Recursively intersect two curves keeping track of their real parameters 
 * and depths of intersection.
 * The results are returned in a 2-D array of doubles indicating the parameters
 * for which intersections are found.  The parameters are in the order the
 * intersections were found, which is probably not in sorted order.
 * When an intersection is found, the parameter value for each of the two 
 * is stored in the index elements array, and the index is incremented.
 * 
 * If either of the curves has subdivisions left before it is straight
 *	(depth > 0)
 * that curve (possibly both) is (are) subdivided at its (their) midpoint(s).
 * the depth(s) is (are) decremented, and the parameter value(s) corresponding
 * to the midpoints(s) is (are) computed.
 * Then each of the subcurves of one curve is intersected with each of the 
 * subcurves of the other curve, first by testing the bounding boxes for
 * interference.  If there is any bounding box interference, the corresponding
 * subcurves are recursively intersected.
 * 
 * If neither curve has subdivisions left, the line segments from the first
 * to last control point of each segment are intersected.  (Actually the 
 * only the parameter value corresponding to the intersection point is found).
 *
 * The apriori flatness test is probably more efficient than testing at each
 * level of recursion, although a test after three or four levels would
 * probably be worthwhile, since many curves become flat faster than their 
 * asymptotic rate for the first few levels of recursion.
 *
 * The bounding box test fails much more frequently than it succeeds, providing
 * substantial pruning of the search space.
 *
 * Each (sub)curve is subdivided only once, hence it is not possible that for 
 * one final line intersection test the subdivision was at one level, while
 * for another final line intersection test the subdivision (of the same curve)
 * was at another.  Since the line segments share endpoints, the intersection
 * is robust: a near-tangential intersection will yield zero or two
 * intersections.
 */
void RecursivelyIntersect( Bezier a, double t0, double t1, int deptha,
			   Bezier b, double u0, double u1, int depthb,
			   std::vector<std::pair<double, double> > &parameters)
{
    if( deptha > 0 )
    {
	Bezier *A = a.Split();
	double tmid = (t0+t1)*0.5;
	deptha--;
	if( depthb > 0 )
        {
	    Bezier *B = b.Split();
	    double umid = (u0+u1)*0.5;
	    depthb--;
	    if( IntersectBB( A[0], B[0] ) )
		RecursivelyIntersect( A[0], t0, tmid, deptha,
				      B[0], u0, umid, depthb,
				      parameters );
	    if( IntersectBB( A[1], B[0] ) )
		RecursivelyIntersect( A[1], tmid, t1, deptha,
				      B[0], u0, umid, depthb,
				      parameters );
	    if( IntersectBB( A[0], B[1] ) )
		RecursivelyIntersect( A[0], t0, tmid, deptha,
				      B[1], umid, u1, depthb,
				      parameters );
	    if( IntersectBB( A[1], B[1] ) )
		RecursivelyIntersect( A[1], tmid, t1, deptha,
				      B[1], umid, u1, depthb,
				      parameters );
        }
	else
        {
	    if( IntersectBB( A[0], b ) )
		RecursivelyIntersect( A[0], t0, tmid, deptha,
				      b, u0, u1, depthb,
				      parameters );
	    if( IntersectBB( A[1], b ) )
		RecursivelyIntersect( A[1], tmid, t1, deptha,
				      b, u0, u1, depthb,
				      parameters );
        }
    }
    else
	if( depthb > 0 )
        {
	    Bezier *B = b.Split();
	    double umid = (u0 + u1)*0.5;
	    depthb--;
	    if( IntersectBB( a, B[0] ) )
		RecursivelyIntersect( a, t0, t1, deptha,
				      B[0], u0, umid, depthb,
				      parameters );
	    if( IntersectBB( a, B[1] ) )
		RecursivelyIntersect( a, t0, t1, deptha,
				      B[0], umid, u1, depthb,
				      parameters );
        }
	else // Both segments are fully subdivided; now do line segments
        {
	    double xlk = a.p[3][X] - a.p[0][X];
	    double ylk = a.p[3][Y] - a.p[0][Y];
	    double xnm = b.p[3][X] - b.p[0][X];
	    double ynm = b.p[3][Y] - b.p[0][Y];
	    double xmk = b.p[0][X] - a.p[0][X];
	    double ymk = b.p[0][Y] - a.p[0][Y];
	    double det = xnm * ylk - ynm * xlk;
	    if( 1.0 + det == 1.0 )
		return;
	    else
            {
		double detinv = 1.0 / det;
		double s = ( xnm * ymk - ynm *xmk ) * detinv;
		double t = ( xlk * ymk - ylk * xmk ) * detinv;
		if( ( s < 0.0 ) || ( s > 1.0 ) || ( t < 0.0 ) || ( t > 1.0 ) )
		    return;
		parameters.push_back(std::pair<double, double>(t0 + s * ( t1 - t0 ),
                                                         u0 + t * ( u1 - u0 )));
            }
        }
}

inline double log4( double x ) { return log(x)/log(4.); }
    
/*
 * Wang's theorem is used to estimate the level of subdivision required,
 * but only if the bounding boxes interfere at the top level.
 * Assuming there is a possible intersection, RecursivelyIntersect is
 * used to find all the parameters corresponding to intersection points.
 * these are then sorted and returned in an array.
 */

Point vabs(Point p) {
    return Point(fabs(p[X]), fabs(p[Y]));
}

std::vector<std::pair<double, double> > FindIntersections( Bezier a, Bezier b)
{
    std::vector<std::pair<double, double> > parameters;
    if( IntersectBB( a, b ) )
    {
	Point la1 = vabs( ( a.p[2] - a.p[1] ) - (a.p[1] - a.p[0]) );
	Point la2 = vabs( ( a.p[3] - a.p[2] ) - (a.p[2] - a.p[1]) );
        double l0 = std::max(std::max(la1[X], la2[X]),
                      std::max(la1[Y], la2[Y]));
	int ra;
	if( l0 * 0.75 * M_SQRT2 + 1.0 == 1.0 ) 
	    ra = 0;
	else
	    ra = (int)ceil( log4( M_SQRT2 * 6.0 / 8.0 * INV_EPS * l0 ) );
	Point lb1 = vabs( (b.p[2] - b.p[1] ) - ( b.p[1] - b.p[0] ) );
	Point lb2 = vabs( (b.p[3] - b.p[2] ) - ( b.p[2] - b.p[1] ) );
	l0 = std::max(std::max(lb1[X], lb2[X]),
                      std::max(lb1[Y], lb2[Y]));
	int rb;
	if( l0 * 0.75 * M_SQRT2 + 1.0 == 1.0 ) 
	    rb = 0;
	else
	    rb = (int)ceil(log4( M_SQRT2 * 6.0 / 8.0 * INV_EPS * l0 ) );
	RecursivelyIntersect( a, 0., 1., ra, b, 0., 1., rb, parameters);
    }
    std::sort(parameters.begin(), parameters.end());
    return parameters;
}

void
Bezier::ParameterSplitLeft(double t, Bezier &left)
{
    left.p[0] = p[0];
    left.p[1] = Lerp(t, p[0], p[1]);
    left.p[2] = Lerp(t, p[1], p[2]); // temporary holding spot
    p[2] = Lerp(t, p[2], p[3]);
    p[1] = Lerp(t, left.p[2], p[2]);
    left.p[2] = Lerp(t, left.p[1], left.p[2]);
    left.p[3] = p[0] = Lerp(t, left.p[2], p[1]);
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

};
