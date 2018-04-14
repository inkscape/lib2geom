#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-geometric.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>

#include <cstdlib>
#include <vector>
#include <list>
#include <algorithm>
using std::vector;
using namespace Geom;

#define SIZE 4
#define NB_SLIDER 2

struct Triangle{
    Point a,b,c;
    double area;
};

//TODO: this would work only for C1 pw<d2<sb>> input. Split the input at corners to work with pwd2sb...
//TODO: for more general purpose, return a path...
void toPoly(D2<SBasis> const &f, std::list<Point> &p, double tol, bool include_first=true){
    D2<SBasis> df = derivative(f);
    D2<SBasis> d2f = derivative(df);
    double t=0;
    if ( include_first ){ p.push_back( f.at0() );}
    while (t<1){
        Point v = unit_vector(df.valueAt(t));
        //OptInterval bounds = bounds_local(df[X]*v[Y]-df[Y]*v[X], Interval(t,1));
        OptInterval bounds = bounds_local(d2f[X]*v[Y]-d2f[Y]*v[X], Interval(t,1));
        if (bounds) {
            double bds_max = (-bounds->min()>bounds->max() ? -bounds->min() : bounds->max());
            double dt;
            //if (bds_max<tol) dt = 1;
            //else            dt = tol/bds_max;
            if (bds_max<tol/4) dt = 1;
            else              dt = 2*std::sqrt( tol / bds_max );
            t+=dt*5;
            if (t>1) t = 1;
        }else{
            t = 1;
        }
        p.push_back( f.valueAt(t) );
    }
    return;
}

std::list<Point> toPoly(std::vector<Piecewise<D2<SBasis> > > f, double tol){
    assert ( f.size() >0 && f[0].size() >0 );
    std::list<Point> res;

    for (unsigned i = 0; i<f.size(); i++){
        for (unsigned j = 0; j<f[i].size(); j++){
            toPoly(f[i][j],res,tol, j==0);
        }
        if ( f[i].segs.front().at0() != f[i].segs.back().at1() ){
            res.push_back( f[i].segs.front().at0() );
        }
        if ( i>0 ) res.push_back( f[0][0].at0() );
    }
    return res;
}

//TODO: this is an ugly hack, use path intersection instead!!
bool intersect(Point const &a0, Point const &b0, Point const &a1, Point const &b1, Point &c, double tol=.0001){
    double abaa1 = cross( b0-a0, a1-a0);
    double abab1 = cross( b0-a0, b1-a0);
    double abaa0 = cross( b1-a1, a0-a1);
    double abab0 = cross( b1-a1, b0-a1);
    if ( abaa1 * abab1 < -tol && abaa0 * abab0 < -tol ){
        c = a1 - (b1-a1) * abaa1/(abab1-abaa1);
        return true;
    }
#if 1
    return false;//TODO: handle limit cases!!
#else
    if ( abaa1 == 0 && dot( a0-a1, b0-a1 ) < 0 ) {
            c = a1;
            return true;
    }
    if ( abab1 == 0 && dot( a0-b1, b0-b1 ) < 0 ) {
            c = b1;
            return true;
    }
    if ( abaa0 == 0 && dot( a1-a0, b1-a0 ) < 0 ) {
            c = a0;
            return true;
    }
    if ( abab0 == 0 && dot( a1-b0, b1-b0 ) < 0 ) {
            c = b0;
            return true;
    }
    return false;
#endif
}

//TODO: use path intersection stuff!
void uncross(std::list<Point> &loop){
    std::list<Point>::iterator b0 = loop.begin(),a0,b1,a1;
    if ( b0 == loop.end() ) return;
    a0 = b0;
    ++b0;
    if ( b0 == loop.end() ) return;
    //now a0,b0 are 2 consecutive points.
    while ( b0 != loop.end() ){
        b1 = b0;
        ++b1;
        if ( b1 != loop.end() ) {
            a1 = b1;
            ++b1;
            if ( b1 != loop.end() ) {
                //now a0,b0,a1,b1 are 4 consecutive points.
                Point c;
                while ( b1 != loop.end() ){
                    if ( intersect(*a0,*b0,*a1,*b1,c) ){
                        if ( c != (*a0) && c != (*b0) ){
                            loop.insert(b1,c);
                            loop.insert(b0,c);
                            ++a1;
                            std::list<Point> loop_piece;
                            loop_piece.insert(loop_piece.begin(), b0, a1 );
                            loop_piece.reverse();
                            loop.erase( b0, a1 );
                            loop.splice( a1, loop_piece );
                            b0 = a0;
                            ++b0;
                            //a1 = b1; a1--;//useless
                        }else{
                            //TODO: handle degenerated crossings...
                        }
                    }else{
                        a1=b1;
                        ++b1;
                    }
                }
            }
        }
        a0 = b0;
        ++b0;
    }
    return;//We should never reach this point.
}
//------------------------------------------------------------
//------------------------------------------------------------
//------------------------------------------------------------
void triangulate(std::list<Point> &pts, std::vector<Triangle> &tri, bool clockwise = false, double tol=.001){
    pts.unique();
    while ( !pts.empty() && pts.front() == pts.back() ){ pts.pop_back(); }
    if ( pts.size() < 3 ) return; 
    //cycle by 1 to have a better looking output...
    pts.push_back(pts.front()); pts.pop_front();
    std::list<Point>::iterator a,b,c,m;
    int sign = (clockwise ? -1 : 1 );
    a = pts.end(); --a;
    b = pts.begin();
    c = b; ++c;
    //now a,b,c are 3 consecutive points.
    if ( pts.size() == 3 ) { 
        Triangle abc;
        abc.a = (*a);
        abc.b = (*b);
        abc.c = (*c);
        abc.area = sign *( cross((*b) - (*a),(*c) - (*b))/2) ;
        if ( abc.area >0 ){
            tri.push_back(abc);        
            pts.clear();
        }
        return; 
    }
    bool found = false;
    while( c != pts.end() ){
        double abac = cross((*b)-(*a),(*c)-(*a));
        if ( fabs(abac)<tol && dot( *b-*a, *c-*b ) <= 0) {
            //this is a degenerated triangle. Remove it and continue. 
            pts.erase(b);
            triangulate(pts,tri,clockwise);
            return;
        }
        m = c;
        ++m;
        while ( m != pts.end() && !found && m!=a){
            bool pointing_inside;
            double abam = cross((*b)-(*a),(*m)-(*a));
            double bcbm = cross((*c)-(*b),(*m)-(*b));
            if ( sign * abac > 0 ){
                pointing_inside = ( sign * abam >= 0 ) && ( sign * bcbm >= 0 );
            }else {
                pointing_inside = ( sign * abam >=0 ) || ( sign * bcbm >=0);
            }
            if ( pointing_inside ){
                std::list<Point>::iterator p=c,q=++p;
                Point inter;
                while ( q != pts.end() && !intersect(*b,*m,*p,*q,inter) ){
                    p=q;
                    ++q;
                }
                if ( q == pts.end() ){
                    found = true;
                }else{
                    ++m;
                }
            }else{
                ++m;
            }
        }
        if ( found ){
            std::list<Point>pts_beg;
            pts.insert(b,*b);
            pts.insert(m,*m);
            pts_beg.splice(pts_beg.begin(), pts, b, m);
            triangulate(pts_beg,tri,clockwise);
            triangulate(pts,tri,clockwise);
            return;
        }else{
            a = b;
            b = c;
            ++c;
        }
    }
    //we should never reach this point.
}

double
my_rand_generator(){
    double x = std::rand();
    return x/RAND_MAX;
}

class RandomGenerator {
public:
    RandomGenerator();
    RandomGenerator(Piecewise<D2<SBasis> >f_in, double tol=.1);
    ~RandomGenerator(){};
    void setDomain(Piecewise<D2<SBasis> >f_in, double tol=.1);
    void set_generator(double (*rand_func)());
    void resetRandomizer();
    Point pt();
    double area();

protected:
    double (*rand)();//set this to your favorite generator of numbers in [0,1] (an inkscape param for instance!)
    long start_seed;
    long seed;
    std::vector<Triangle> triangles;
    std::vector<double> areas;
};

RandomGenerator::RandomGenerator(){
    seed = start_seed = 10;
    rand = &my_rand_generator;//set this to your favorite generator of numbers in [0,1]!
}
RandomGenerator::RandomGenerator(Piecewise<D2<SBasis> >f_in, double tol){
    seed = start_seed = 10;
    rand = &my_rand_generator;//set this to your favorite generator of numbers in [0,1]!
    setDomain(f_in, tol);
}
void RandomGenerator::setDomain(Piecewise<D2<SBasis> >f_in, double tol){
    std::vector<Piecewise<D2<SBasis> > >f = split_at_discontinuities(f_in);
    std::list<Point> p = toPoly( f, tol);
    uncross(p);
    if ( p.size()<3) return;
    double tot_area = 0;
    std::list<Point>::iterator a = p.begin(), b=a;
    ++b;
    while(b!=p.end()){
        tot_area += ((*b)[X]-(*a)[X]) * ((*b)[Y]+(*a)[Y])/2;
        ++a;++b;
    }
    bool clockwise = tot_area < 0;
    triangles = std::vector<Triangle>();
    triangulate(p,triangles,clockwise);
    areas = std::vector<double>(triangles.size(),0.);
    double cumul = 0;
    for (unsigned i = 0; i<triangles.size(); i++){
        cumul += triangles[i].area;
        areas[i] = cumul;
    }
}

void RandomGenerator::resetRandomizer(){
    seed = start_seed;
}
Point RandomGenerator::pt(){
    if (areas.empty()) return Point(0,0);
    double pick_area = rand()*areas.back();
    std::vector<double>::iterator picked = std::lower_bound( areas.begin(), areas.end(), pick_area);
    unsigned i = picked - areas.begin();
    double x = (*rand)();
    double y = (*rand)();
    if ( x+y > 1) {
        x = 1-x;
        y = 1-y;
    }
    //x=.3; y=.3;
    Point res;
    res = triangles[i].a;
    res += x * ( triangles[i].b - triangles[i].a );
    res += y * ( triangles[i].c - triangles[i].a );
    return res;
}
double RandomGenerator::area(){
    if (areas.empty()) return 0;
    return areas.back();
}
void RandomGenerator::set_generator(double (*f)()){
    rand = f;//set this to your favorite generator of numbers in [0,1]!
}






//-------------------------------------------------------
// The toy!
//-------------------------------------------------------
class RandomToy: public Toy {

    PointHandle adjuster[NB_SLIDER];

public:
    PointSetHandle b1_handle;
    PointSetHandle b2_handle;
    void draw(cairo_t *cr,
	      std::ostringstream *notify,
	      int width, int height, bool save, std::ostringstream *timer_stream) override {
        srand(10);
        for(unsigned i=0; i<NB_SLIDER; i++){
            adjuster[i].pos[X] = 30+i*20;
            if (adjuster[i].pos[Y]<100) adjuster[i].pos[Y] = 100;
            if (adjuster[i].pos[Y]>400) adjuster[i].pos[Y] = 400;
            cairo_move_to(cr, Point(30+i*20,100));
            cairo_line_to(cr, Point(30+i*20,400));
            cairo_set_line_width (cr, .5);
            cairo_set_source_rgba (cr, 0., 0., 0., 1);
            cairo_stroke(cr);
        }
        double tol     = (400-adjuster[0].pos[Y])/300.*5+0.05;
        double tau     = (400-adjuster[1].pos[Y])/300.;
//         double scale_topback  = (250-adjuster[2].pos[Y])/150.*5;
//         double scale_botfront = (250-adjuster[3].pos[Y])/150.*5;
//         double scale_botback  = (250-adjuster[4].pos[Y])/150.*5;
//         double growth =       1+(250-adjuster[5].pos[Y])/150.*.1;
//         double rdmness =      1+(400-adjuster[6].pos[Y])/300.*.9;
//         double bend_amount    = (250-adjuster[7].pos[Y])/300.*100.;

        b1_handle.pts.back() = b2_handle.pts.front(); 
        b1_handle.pts.front() = b2_handle.pts.back(); 
        D2<SBasis> B1 = b1_handle.asBezier();
        D2<SBasis> B2 = b2_handle.asBezier();
        
        cairo_set_line_width(cr, 0.3);
        cairo_set_source_rgba(cr, 0, 0, 0, 1);
        cairo_d2_sb(cr, B1);
        cairo_d2_sb(cr, B2);
        cairo_set_line_width (cr, .5);
        cairo_set_source_rgba (cr, 0., 0., 0., 1);
        cairo_stroke(cr);

        
        Piecewise<D2<SBasis> >B;
        B.concat(Piecewise<D2<SBasis> >(B1));
        B.continuousConcat(Piecewise<D2<SBasis> >(B2));

        Piecewise<SBasis> are;
        
        Point centroid_tmp(0,0);
        are = integral(dot(B, rot90(derivative(B))))*0.5;
        are = (are - are.firstValue())*(height/10) / (are.lastValue() - are.firstValue());
    
        D2<Piecewise<SBasis> > are_graph(Piecewise<SBasis>(Linear(0, width)), are );
        std::cout << are.firstValue() << "," << are.lastValue() << std::endl;
        cairo_save(cr);
        cairo_d2_pw_sb(cr, are_graph);
        cairo_set_line_width (cr, .5);
        cairo_set_source_rgba (cr, 0., 0., 0., 1);
        cairo_stroke(cr);
        cairo_restore(cr);
        

#if 0
        std::vector<Piecewise<D2<SBasis> > >f = split_at_discontinuities(B);
        std::list<Point> p = toPoly( f, tol);
        uncross(p);
        cairo_move_to(cr, p.front());
        for (std::list<Point>::iterator pt = p.begin(); pt!=p.end(); ++pt){
            cairo_line_to(cr, *pt);
            //if (i++>p.size()*tau) break;
        } 
        cairo_set_line_width (cr, 3);
        cairo_set_source_rgba (cr, 1., 0., 0., .5);
        cairo_stroke(cr);

        if ( p.size()<3) return;
        double tot_area = 0;
        std::list<Point>::iterator a = p.begin(), b=a;
        b++;
        while(b!=p.end()){
            tot_area += ((*b)[X]-(*a)[X]) * ((*b)[Y]+(*a)[Y])/2;
            a++;b++;
        }
        bool clockwise = tot_area < 0;

        std::vector<Triangle> tri;
        int nbiter =0;
        triangulate(p,tri,clockwise);
        cairo_set_source_rgba (cr, 1., 1., 0., 1);
        cairo_stroke(cr);
        for (unsigned i=0; i<tri.size(); i++){
            cairo_move_to(cr, tri[i].a);
            cairo_line_to(cr, tri[i].b);
            cairo_line_to(cr, tri[i].c);
            cairo_line_to(cr, tri[i].a);
            cairo_set_line_width (cr, .5);
            cairo_set_source_rgba (cr, 0., 0., .9, .5);
            cairo_stroke(cr);
            cairo_move_to(cr, tri[i].a);
            cairo_line_to(cr, tri[i].b);
            cairo_line_to(cr, tri[i].c);
            cairo_line_to(cr, tri[i].a);
            cairo_set_source_rgba (cr, 0.5, 0., .9, .1);
            cairo_fill(cr);
        } 
#endif
 
        RandomGenerator rdm = RandomGenerator(B, tol);
        for(int i = 0; i < rdm.area()/5*tau; i++) {
            draw_handle(cr, rdm.pt());
        }
        cairo_set_source_rgba (cr, 0., 0., 0., 1);
        cairo_stroke(cr);

        Toy::draw(cr, notify, width, height, save,timer_stream);
    }        
  
public:
    RandomToy(){
        for(int i = 0; i < SIZE; i++) {
            b1_handle.push_back(150+uniform()*300,150+uniform()*300);
            b2_handle.push_back(150+uniform()*300,150+uniform()*300);
        }
        b1_handle.pts[0] = Geom::Point(400,300);
        b1_handle.pts[1] = Geom::Point(400,400);
        b1_handle.pts[2] = Geom::Point(100,400);
        b1_handle.pts[3] = Geom::Point(100,300);

        b2_handle.pts[0] = Geom::Point(100,300);
        b2_handle.pts[1] = Geom::Point(100,200);
        b2_handle.pts[2] = Geom::Point(400,200);
        b2_handle.pts[3] = Geom::Point(400,300);
        handles.push_back(&b1_handle);
        handles.push_back(&b2_handle);

        for(unsigned i = 0; i < NB_SLIDER; i++) {
            adjuster[i].pos = Geom::Point(30+i*20,250);
            handles.push_back(&(adjuster[i]));
        }
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new RandomToy);
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
//vim:filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99:
