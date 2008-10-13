#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-2d.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-geometric.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

#include <time.h>
using std::vector;
using namespace Geom;
using namespace std;


// TODO: 

#include <gsl/gsl_poly.h>

using std::vector;
using namespace Geom;

#define ZERO 1e-4

//==================================================================
static vector<double>  solve_poly (double a[],unsigned deg){
    double tol=1e-7;
    vector<double> result;
    int i;
    i=deg;
    while( i>=0 && fabs(a[i])<tol ) i--;
    deg=i;
    
    double z[2*deg];
    gsl_poly_complex_workspace * w 
        = gsl_poly_complex_workspace_alloc (deg+1);
    gsl_poly_complex_solve (a, deg+1, w, z);
    gsl_poly_complex_workspace_free (w);
    
    //collect real solutions.
    for (unsigned i = 0; i < deg; i++){
        if (fabs(z[2*i+1])<tol){
            result.push_back(z[2*i]);
        }
    }
    return result;
}


//TODO: handle multiple solutions/non existence/0 division cases
//TODO: clean up, then move this into 2Geom core.
//
//returns the cubic fitting direction and curvatrue of a given
// input curve at two points.
// The input can be the 
//    value, speed, and acceleration
// or
//    value, speed, and cross(acceleration,speed) 
// of the original curve at the both ends.
static std::vector<D2<SBasis> >
cubics_fitting_curvature(Point const &M0,   Point const &M1,
                         Point const &dM0,  Point const &dM1,
                         double d2M0xdM0,  double d2M1xdM1,
                         int insist_on_speed_signs = 1){
    std::vector<D2<SBasis> > result;

    //speed of cubic bezier will be lambda0*dM0 and lambda1*dM1,
    //with lambda0 and lambda1 s.t. curvature at both ends is the same
    //as the curvature of the given curve.
    std::vector<double> lambda0,lambda1;
    double dM1xdM0=cross(dM1,dM0);
    if (fabs(dM1xdM0)<ZERO){
        //TODO: if d2M0xdM0==0 or d2M1xdM1==0: no solution!
        lambda0.push_back(sqrt( 6*cross(M1-M0,dM0)/d2M0xdM0) );
        lambda1.push_back(sqrt(-6*cross(M1-M0,dM1)/d2M1xdM1) );
    }else{
        //solve:  lambda1 = a0 lambda0^2 + c0
        //        lambda0 = a1 lambda1^2 + c1
        double a0,c0,a1,c1;
        a0 = -d2M0xdM0/2/dM1xdM0;
        c0 =  3*cross(M1-M0,dM0)/dM1xdM0;
        a1 = -d2M1xdM1/2/dM1xdM0;
        c1 = -3*cross(M1-M0,dM1)/dM1xdM0;

        if (fabs(a0)<ZERO){//TODO: if !insist_on_speed_signs, more sols to collect!
            lambda1.push_back( c0 );
            lambda0.push_back( a1*c0*c0 + c1 );
        }else if (fabs(a1)<ZERO){
            lambda0.push_back( c1 );
            lambda1.push_back( a0*c1*c1 + c0 );
        }else{
            //find lamda0 by solving a deg 4 equation d0+d1*X+...+d4*X^4=0
            double a[5];
            a[0] = c1+a1*c0*c0;
            a[1] = -1;
            a[2] = 2*a1*a0*c0;
            a[3] = 0;
            a[4] = a1*a0*a0;
            vector<double> solns=solve_poly(a,4);
            for (unsigned i=0;i<solns.size();i++){
                double lbda0=solns[i];
                double lbda1=c0+a0*lbda0*lbda0;
                //only keep solutions pointing in the same direction...
                if (lbda0>=0. && lbda1>=0.){
                    lambda0.push_back( lbda0);
                    lambda1.push_back( lbda1);
                }
                else if (lbda0<=0. && lbda1<=0. && insist_on_speed_signs<=0){
                    lambda0.push_back( lbda0);
                    lambda1.push_back( lbda1);
                }
                else if (insist_on_speed_signs<0){
                    lambda0.push_back( lbda0);
                    lambda1.push_back( lbda1);
                }
            }
        }
    }
    
    for (unsigned i=0; i<lambda0.size(); i++){
        Point V0 = lambda0[i]*dM0;
        Point V1 = lambda1[i]*dM1;
        D2<SBasis> cubic;
        for(unsigned dim=0;dim<2;dim++){
            cubic[dim] = Linear(M0[dim],M1[dim]);
            cubic[dim].push_back(Linear( M0[dim]-M1[dim]+V0[dim],
                                         -M0[dim]+M1[dim]-V1[dim]));
        }
#if 0
        Piecewise<SBasis> k = curvature(result);
        double dM0_l = dM0.length();
        double dM1_l = dM1.length();
        g_warning("Target radii: %f, %f", dM0_l*dM0_l*dM0_l/d2M0xdM0,dM1_l*dM1_l*dM1_l/d2M1xdM1);
        g_warning("Obtained radii: %f, %f",1/k.valueAt(0),1/k.valueAt(1));
#endif
        result.push_back(cubic);
    }
    return(result);
}
static std::vector<D2<SBasis> >
cubics_fitting_curvature(Point const &M0,   Point const &M1,
                         Point const &dM0,  Point const &dM1,
                         Point const &d2M0, Point const &d2M1,
                         int insist_on_speed_signs = 1){
    double d2M0xdM0 = cross(d2M0,dM0);
    double d2M1xdM1 = cross(d2M1,dM1);
    return cubics_fitting_curvature(M0,M1,dM0,dM1,d2M0xdM0,d2M1xdM1,insist_on_speed_signs);
}

//-----------------------------------------------

class CurvatureTester: public Toy {
    PointSetHandle curve_handle;
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
    
        D2<SBasis> B = curve_handle.asBezier();

        cairo_set_line_width (cr, 1);
        cairo_set_source_rgba (cr, 0., 0.5, 0., 1);

	for(int i = 0; i < 2; i++) {
            Geom::Point center=curve_handle.pts[1+i];
            Geom::Point normal=center- curve_handle.pts[3*i];
            double radius = Geom::L2(normal);
            *notify<<"K="<<radius<<std::endl;
            if (fabs(radius)>1e-4){
	    
                cairo_arc(cr, center[0], center[1],fabs(radius), 0, M_PI*2);
                draw_handle(cr, center);
            }else{
            }
            cairo_set_source_rgba (cr, 0.5, 0.2, 0., 0.8);
            cairo_stroke(cr);
	}
	if (1) {
            Geom::Point A = curve_handle.pts[0];
            Geom::Point B = curve_handle.pts[3];
            Geom::Point dA = curve_handle.pts[1]-A;
            Geom::Point dB = B-curve_handle.pts[2];
            std::vector<D2<SBasis> > candidates = 
                cubics_fitting_curvature(curve_handle.pts[0],curve_handle.pts[3],
                                         rot90(dA),
                                         rot90(dB),
                                         -L2sq(dA), L2sq(dB));
	  
            if (candidates.size()==0) {
                cairo_md_sb(cr,  D2<SBasis>(Linear(A[X],B[X]),Linear(A[Y],B[Y])));
                cairo_stroke(cr);
            } else {
                //TODO: I'm sure std algorithm could do that for me...
                double error = -1;
                unsigned best = 0;
                for (unsigned i=0; i<candidates.size(); i++){
                    double l = length(candidates[i]);
	  
                    if ( l < error || error < 0 ){
                        error = l;
                        best = i;
                    }
                }
                cairo_md_sb(cr, candidates[best]);
                cairo_stroke(cr);
            }
        }
        Toy::draw(cr, notify, width, height, save);
    }        
  
    void canvas_click(Geom::Point at, int button) {
        std::cout << "clicked at:" << at << " with button " << button << std::endl;
        if(button == 1) {
            curve_handle.push_back(at+Point(100,100));
            curve_handle.push_back(at);
        }
    }

public:
    CurvatureTester(){
        if(handles.empty()) {
            handles.push_back(&curve_handle);
            for(unsigned i = 0; i < 4; i++)
                curve_handle.push_back(150+uniform()*300,150+uniform()*300);
        }
    }
};

int main(int argc, char **argv) {
    std::cout << "testing unit_normal(multidim_sbasis) based offset." << std::endl;
    init(argc, argv, new CurvatureTester);
    return 0;
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
//vim:expandtab:shiftwidth = 4:tabstop = 8:softtabstop = 4:encoding = utf-8:textwidth = 99 :

 	  	 
