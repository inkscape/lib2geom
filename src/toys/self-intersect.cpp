#include "s-basis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "multidim-sbasis-bounds.h"
#include "s-basis-2d.h"

#include "path-cairo.h"

#include <iterator>
#include "translate.h"

#include "toy-framework.h"

using std::vector;
const unsigned bez_ord = 10;
using namespace Geom;
cairo_t *g_cr = 0;
const double eps = 0.1;

class Bezier {
public:
    std::vector<Geom::Point> p;
    Bezier() {
    }
    Bezier * split(double t = 0.5);
    
    ~Bezier() {}

    void bounds(double &minax, double &maxax, 
                double &minay, double &maxay) {
        // Compute bounding box for a
        minax = p[0][X];	 // These are the most likely to be extremal
        maxax = p.back()[X];
        if( minax > maxax )
            swap(minax, maxax);
        for(unsigned i = 1; i < p.size()-1; i++) {
            if( p[i][X] < minax )
                minax = p[i][X];
            else if( p[i][X] > maxax )
                maxax = p[i][X];
        }

        minay = p[0][Y];	 // These are the most likely to be extremal
        maxay = p.back()[Y];
        if( minay > maxay )
            swap(minay, maxay);
        for(unsigned i = 1; i < p.size()-1; i++) {
            if( p[i][Y] < minay )
                minay = p[i][Y];
            else if( p[i][Y] > maxay )
                maxay = p[i][Y];
        }

    }
    
};

std::vector<std::pair<double, double> > find_intersections( Bezier a, Bezier b);

class PairIntersect: public Toy {
virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
    cairo_set_line_width (cr, 0.5);
    cairo_set_source_rgba (cr, 0., 0., 0, 1);
    
    MultidimSBasis<2> A = bezier_to_sbasis<2, bez_ord-1>(handles.begin());
    cairo_md_sb(cr, A);
    cairo_stroke(cr);
    vector<double> dxr = roots(derivative(A[X]));
    vector<double> dyr = roots(derivative(A[Y]));
    
    dxr.insert(dxr.begin(), dyr.begin(), dyr.end());
    dxr.push_back(0);
    dxr.push_back(1);
    sort(dxr.begin(), dxr.end());
    unique(dxr.begin(), dxr.end());
    
    Rect Ar = local_bounds(A, 0, 1);
    std::vector<std::pair<double, double> > all_si;
    Bezier Sb;
    Sb.p.insert(Sb.p.begin(), handles.begin(), handles.begin()+bez_ord);
    vector<Bezier> pieces;
    for(int i = 0; i < dxr.size()-1; i++) {
        Bezier* r = Sb.split((dxr[i+1]-dxr[i]) / (1 - dxr[i]));
        pieces.push_back(r[0]);
        Sb = r[1];
    }
    for(int i = 0; i < dxr.size()-1; i++) {

        
        for(int j = i+1; j < dxr.size()-1; j++) {
            std::vector<std::pair<double, double> > section = 
                find_intersections( pieces[i], pieces[j]);
            for(int k = 0; k < section.size(); k++) {
                double l = section[k].first;
                double r = section[k].second;
                if(l == 1) continue;
                if(r == 1) continue;
                all_si.push_back(make_pair((1-l)*dxr[i] + l*dxr[i+1],
                                           (1-r)*dxr[j] + r*dxr[j+1]));
            }
        }
    }
    
    sort(all_si.begin(), all_si.end());
    unique(all_si.begin(), all_si.end());
    
    cairo_stroke(cr);
    if(0) {
        cairo_set_source_rgba (cr, 1., 0., 0, 1);
        for(int i = 0; i < dxr.size(); i++) {
            draw_handle(cr, point_at(A, dxr[i]));
        }
        cairo_stroke(cr);
    }
    cairo_set_source_rgba (cr, 1., 0., 1, 1);
    for(int i = 0; i < all_si.size(); i++) {
        draw_handle(cr, point_at(A, all_si[i].first));
    }
    cairo_stroke(cr);

    Toy::draw(cr, notify, width, height, save);
}
public:
PairIntersect () {
    for(unsigned i = 0; i < bez_ord; i++)
        handles.push_back(Geom::Point(uniform()*400, uniform()*400));
}
};

int main(int argc, char **argv) {   
    init(argc, argv, "Pair Intersect", new PairIntersect());

    return 0;
}


/* The value of 1.0 / (1L<<14) is enough for most applications */
const double INV_EPS = (1L<<14);
    
/*
 * split the curve at the midpoint, returning an array with the two parts
 * Temporary storage is minimized by using part of the storage for the result
 * to hold an intermediate value until it is no longer needed.
 */
#define left r[0]
#define right r[1]
Bezier *Bezier::split(double t) {
    Bezier *r = new Bezier[2];
    const unsigned sz = p.size();
    Geom::Point Vtemp[sz][sz];

    /* Copy control points	*/
    std::copy(p.begin(), p.end(), Vtemp[0]);

    /* Triangle computation	*/
    for (unsigned i = 1; i < sz; i++) {	
        for (unsigned j = 0; j < sz - i; j++) {
            Vtemp[i][j] = Lerp(t, Vtemp[i-1][j], Vtemp[i-1][j+1]);
        }
    }
    
    r[0].p.resize(sz);
    r[1].p.resize(sz);
    for (unsigned j = 0; j < sz; j++)
        r[0].p[j]  = Vtemp[j][0];
    for (unsigned j = 0; j < sz; j++)
        r[1].p[j] = Vtemp[sz-1-j][j];

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

bool intersect_BB( Bezier a, Bezier b ) {
    double minax, maxax, minay, maxay;
    a.bounds(minax, maxax, minay, maxay);
    double minbx, maxbx, minby, maxby;
    b.bounds(minbx, maxbx, minby, maxby);
    // Test bounding box of b against bounding box of a
    // Not >= : need boundary case
    return not( ( minax > maxbx ) || ( minay > maxby )
                || ( minbx > maxax ) || ( minby > maxay ) );
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
void recursively_intersect( Bezier a, double t0, double t1, int deptha,
			   Bezier b, double u0, double u1, int depthb,
			   std::vector<std::pair<double, double> > &parameters)
{
    if( deptha > 0 )
    {
	Bezier *A = a.split();
	double tmid = (t0+t1)*0.5;
	deptha--;
	if( depthb > 0 )
        {
	    Bezier *B = b.split();
	    double umid = (u0+u1)*0.5;
	    depthb--;
	    if( intersect_BB( A[0], B[0] ) )
		recursively_intersect( A[0], t0, tmid, deptha,
				      B[0], u0, umid, depthb,
				      parameters );
	    if( intersect_BB( A[1], B[0] ) )
		recursively_intersect( A[1], tmid, t1, deptha,
				      B[0], u0, umid, depthb,
				      parameters );
	    if( intersect_BB( A[0], B[1] ) )
		recursively_intersect( A[0], t0, tmid, deptha,
				      B[1], umid, u1, depthb,
				      parameters );
	    if( intersect_BB( A[1], B[1] ) )
		recursively_intersect( A[1], tmid, t1, deptha,
				      B[1], umid, u1, depthb,
				      parameters );
        }
	else
        {
	    if( intersect_BB( A[0], b ) )
		recursively_intersect( A[0], t0, tmid, deptha,
				      b, u0, u1, depthb,
				      parameters );
	    if( intersect_BB( A[1], b ) )
		recursively_intersect( A[1], tmid, t1, deptha,
				      b, u0, u1, depthb,
				      parameters );
        }
    }
    else
	if( depthb > 0 )
        {
	    Bezier *B = b.split();
	    double umid = (u0 + u1)*0.5;
	    depthb--;
	    if( intersect_BB( a, B[0] ) )
		recursively_intersect( a, t0, t1, deptha,
				      B[0], u0, umid, depthb,
				      parameters );
	    if( intersect_BB( a, B[1] ) )
		recursively_intersect( a, t0, t1, deptha,
				      B[0], umid, u1, depthb,
				      parameters );
        }
	else // Both segments are fully subdivided; now do line segments
        {
	    double xlk = a.p.back()[X] - a.p[0][X];
	    double ylk = a.p.back()[Y] - a.p[0][Y];
	    double xnm = b.p.back()[X] - b.p[0][X];
	    double ynm = b.p.back()[Y] - b.p[0][Y];
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
 * Assuming there is a possible intersection, recursively_intersect is
 * used to find all the parameters corresponding to intersection points.
 * these are then sorted and returned in an array.
 */

double Lmax(Point p) {
    return std::max(fabs(p[X]), fabs(p[Y]));
}

unsigned wangs_theorem(Bezier a) {
    return 12; // seems a good approximation!
    double la1 = Lmax( ( a.p[2] - a.p[1] ) - (a.p[1] - a.p[0]) );
    double la2 = Lmax( ( a.p[3] - a.p[2] ) - (a.p[2] - a.p[1]) );
    double l0 = std::max(la1, la2);
    int ra;
    if( l0 * 0.75 * M_SQRT2 + 1.0 == 1.0 ) 
        ra = 0;
    else
        ra = (int)ceil( log4( M_SQRT2 * 6.0 / 8.0 * INV_EPS * l0 ) );
    cout << ra << std::endl;
    return ra;
}

std::vector<std::pair<double, double> > find_intersections( Bezier a, Bezier b)
{
    std::vector<std::pair<double, double> > parameters;
    if( intersect_BB( a, b ) )
    {
	recursively_intersect( a, 0., 1., wangs_theorem(a), 
                               b, 0., 1., wangs_theorem(b), 
                               parameters);
    }
    std::sort(parameters.begin(), parameters.end());
    return parameters;
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
