#include <iostream>
#include <2geom/path.h>
#include <2geom/svg-path-parser.h>
#include <2geom/path-intersection.h>
#include <2geom/basic-intersection.h>
#include <2geom/pathvector.h>

#include <cstdlib>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>
#include <2geom/ord.h>
using namespace Geom;
using namespace std;


// class IntersectionData {
//     class IntersectionBox{
//         vector<pair<int, int> > boundary; // pairs of area index and ray-curve index
//         Geom::Rect bounds;
//     };
    
//     class Area {
//         vector<pair<int, int> > boundary; // pairs of intersectbox index and bdry-curve index
//         int winding;
//     };
    
//     class Curve {
//         int left, right; // the area index to the left and to the right of this curve
//         Geom::Curve parent;
//         Geom::Interval parent_portion;
//         Geom::Rect bounds;
//     };
    
//     vector<Area> areas;
//     vector<Curve> curves;
//     vector<IntersectionBox> intersections;
// };

//Returns the first non zero coeff of the monomial exp of f at t.
//TODO: make this less expensive!
unsigned valuationAt(SBasis f, double t, double &first, double tol = .001){
    if ( f.size()>0 ){
        first = f.valueAt(t);
        if ( fabs(first) > tol ){
            return 0;
        }else{
            unsigned res = valuationAt(derivative (f), t, first, tol ) + 1;
            if (res > 0) first /= res;
            return res;
        }
    }
    first  = 0;
    return -1;//FIXME: how do you say +infty?
}
//Returns the deg (and value in &first) of the first non zero coeff of the monomial exp of f at t.
//TODO: make this less expensive!
unsigned valuationAt(D2<SBasis> f, double t, Point &first, double tol = .001){
    if ( f[X].size()>0 or f[Y].size()>0 ){
        first = f.valueAt(t);
        if ( L2(first) > tol ){
            return 0;
        }else{
            unsigned res = valuationAt(derivative (f), t, first, tol ) + 1;
            if (res > 0) first /= res;
            return res;
        }
    }
    first = Point(0,0);
    return -1;//FIXME: how do you say +infty?
}

//Warning: Rays should not be constant = origin!!
struct RayOrder{
    Point start;
    bool clockwise;
    double tol;
    RayOrder() : start(-1,0), clockwise(false), tol(0.001) {}
    RayOrder(Point dir, double eps=.01) : start(unit_vector(dir)), clockwise(false), tol(eps) {}
    bool operator()(D2<SBasis> a, D2<SBasis> b) {
        D2<SBasis> da = derivative( a );
        D2<SBasis> db = derivative( b );
        Point dpa, dpb;
        unsigned p_a = valuationAt(da,0,dpa,tol)+1;
        unsigned p_b = valuationAt(db,0,dpb,tol)+1;
        std::cout<<"p_a = "<<p_a<<", p_b = "<<p_b<<"\n";
        assert ( L2(dpa) != 0 && L2(dpb) != 0 );

        Matrix m;
        m.setXAxis(-start);
        m.setYAxis(-rot90(start));
        m = m.inverse();
        Point dir_a = dpa*m;
        Point dir_b = dpb*m;
        double angle_a = atan2(dir_a);
        double angle_b = atan2(dir_b);

        std::cout<<"angle_a - angle_b = "<< angle_a<<" - "<<angle_b<<" = "<<angle_a-angle_b<<"\n";        
        if ( angle_a < angle_b - tol ) return true;
        if ( angle_a > angle_b + tol) return false;

        //If we're still here, a and b have the same tangent slopes at 0.
        //now make things "straight" to get the '(t^p,t^q)' leading terms.
        Matrix m_dir;
        m_dir.setXAxis(dpa);
        m_dir.setYAxis(rot90(dpa));
        m_dir = m_dir.inverse();
        D2<SBasis> da_std = da * m_dir;
        D2<SBasis> db_std = db * m_dir;
        double wa, wb;
        unsigned q_a = valuationAt(da_std[Y], 0, wa, tol)+1; 
        unsigned q_b = valuationAt(db_std[Y], 0, wb, tol)+1; 
        double main_ay = wa / pow( L2(dpa), double(q_a)/p_a );
        double main_by = wb / pow( L2(dpb), double(q_b)/p_b );
        if ( q_a/p_a < q_b/p_b ) return (main_by>0);
        if ( q_a/p_a > q_b/p_b ) return (main_ay<0);
        if ( main_by > main_ay ) return true;
        if ( main_by < main_ay ) return false;

        //If we're still here, a and b have the same first (v*t^p,w*t^q) terms.
        //if we want to go deeper:
        // -reparametrize to have x=t^p:
        // (write x=t^p+t^p+1+... = t^p(1+t+...) = ( t*(1+...)^1/p )^p
        //  and take T = t*(1+...)^1/p).
        // -compute the Taylor expansion of both y5T) until one is different from the other!
        //
        //The point is that I fear we'll quickly go beyond any meaningfull precision...
        std::cout<<"hm. could not decide!\n";
        return false;
    }
};


#define NB_RAYS 5
class RaySortingTest: public Toy {
    PointSetHandle bez_handle[NB_RAYS];
    Point origin;
    Point tangent;

    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        cairo_set_source_rgba (cr, 0., 0., 0, 1);
        cairo_set_line_width (cr, 1);
        
        std::vector<D2<SBasis> > rays;
        rays.resize(NB_RAYS);
        for (int i = 0; i < NB_RAYS; i++){
            bez_handle[i].pts[0] = origin;
            bez_handle[i].pts[1][Y] = (origin+tangent)[Y];
            rays[i] = bez_handle[i].asBezier();
        }
        
        sort(rays.begin(), rays.end(), RayOrder() );

        for (int i = 0; i < NB_RAYS; i++){
            cairo_set_source_rgba (cr, 1-1./NB_RAYS*i, 0., 1./NB_RAYS*i, 1);
            cairo_set_line_width (cr, 1);
            cairo_d2_sb(cr, rays[i]);
            draw_number(cr, rays[i].at1(),i);
            cairo_stroke(cr);
        }


        Toy::draw(cr, notify, width, height, save,timer_stream);
    }

    public:
    RaySortingTest (): origin(300,300), tangent(100,0){
        for (int i = 0; i < NB_RAYS; i++){
            bez_handle[i] = PointSetHandle();
        }
    }
    virtual bool should_draw_numbers() { return false; }
    void first_time(int argc, char** argv) {
        origin = Point(150,300);
        tangent = Point(100,0);
        for(int i = 0; i < NB_RAYS; i++){
            double h = (uniform()-.5)*4;
            Point pt = origin;
            bez_handle[i].push_back(pt[X],pt[Y]);
            pt += tangent;
            bez_handle[i].push_back(pt[X],pt[Y]);
            pt += tangent + h*rot90(tangent);
            bez_handle[i].push_back(pt[X],pt[Y]);
            pt += tangent;
            bez_handle[i].push_back(pt[X],pt[Y]);
//             double angle = uniform()*2*3.1415;
//             Point dir( cos(angle), sin(angle) );
//             for(int j = 0; j < 4; j++){
//                 bez_handle[i].push_back(origin[X]+j*50*dir[X], origin[Y]+j*50*dir[Y]);
//             }
            handles.push_back(&bez_handle[i]);
        }
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new RaySortingTest());
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
