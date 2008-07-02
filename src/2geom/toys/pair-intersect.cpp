#include <2geom/basic-intersection.h>
#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/bezier-to-sbasis.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

using std::vector;
using namespace Geom;

cairo_t *g_cr = 0;
const double eps = 0.1;

extern unsigned intersect_steps;

#if 0
/** Given two linear md_sb(assume they are linear even if they're not)
    find the ts at the intersection. */
bool
linear_pair_intersect(D2<SBasis> A, double Al, double Ah, 
                      D2<SBasis> B, double Bl, double Bh,
                      double &tA, double &tB) {
    Rect Ar = bounds_local(A, Interval(Al, Ah));
    cairo_rectangle(g_cr, Ar.min()[0], Ar.min()[1], Ar.max()[0], Ar.max()[1]);
    cairo_stroke(g_cr);
    std::cout << Al << ", " << Ah << "\n";
    // kramers rule here
    Point A0 = A(Al);
    Point A1 = A(Ah);
    Point B0 = B(Bl);
    Point B1 = B(Bh);
    double xlk = A1[X] - A0[X];
    double ylk = A1[Y] - A0[Y];
    double xnm = B1[X] - B0[X];
    double ynm = B1[Y] - B0[Y];
    double xmk = B0[X] - A0[X];
    double ymk = B0[Y] - A0[Y];
    double det = xnm * ylk - ynm * xlk;
    if( 1.0 + det == 1.0 )
        return false;
    else
    {
        double detinv = 1.0 / det;
        double s = ( xnm * ymk - ynm *xmk ) * detinv;
        double t = ( xlk * ymk - ylk * xmk ) * detinv;
        if( ( s < 0.0 ) || ( s > 1.0 ) || ( t < 0.0 ) || ( t > 1.0 ) )
            return false;
        tA = Al + s * ( Ah - Al );
        tB = Bl + t * ( Bh - Bl );
        return true;
    }
}

void pair_intersect(vector<double> &Asects,
                    vector<double> &Bsects,
                    D2<SBasis> A, double Al, double Ah, 
                    D2<SBasis> B, double Bl, double Bh, unsigned depth=0) {
    // we'll split only A, and swap args
    Rect Ar = bounds_local(A, Interval(Al, Ah));
    if(Ar.isEmpty()) return;

    Rect Br = bounds_local(B, Interval(Bl, Bh));
    if(Br.isEmpty()) return;
    
    if((depth > 12) || Ar.intersects(Br)) {
        double Ate = 0;
        double Bte = 0;
        for(unsigned d = 0; d < 2; d++) {
            Interval bs = bounds_local(A[d], Interval(Al, Ah), 1); //only 1?
            Ate = std::max(Ate, bs.extent());
        }
        for(unsigned d = 0; d < 2; d++) {
            Interval bs = bounds_local(B[d], Interval(Bl, Bh), 1);
            Bte = std::max(Bte, bs.extent());
        }

        if((depth > 12)  || ((Ate < eps) && 
           (Bte < eps))) {
            std::cout << "intersects\n" << Ate << "\n" << Bte;
            double tA, tB;
            if(linear_pair_intersect(A, Al, Ah, 
                                     B, Bl, Bh, 
                                     tA, tB)) {
                Asects.push_back(tA);
                Bsects.push_back(tB);
            }
            
        } else {
            double mid = (Al + Ah)/2;
            pair_intersect(Bsects,
                           Asects,
                           B, Bl, Bh,
                           A, Al, mid, depth+1);
            pair_intersect(Bsects,
                           Asects,
                           B, Bl, Bh,
                           A, mid, Ah, depth+1);
        }
    }
}

#endif

class PairIntersect: public Toy {
    PointSetHandle A_handles;
    PointSetHandle B_handles;
virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
    cairo_set_line_width (cr, 0.5);
    
    D2<SBasis> A = A_handles.asBezier();
    cairo_md_sb(cr, A);
    
    D2<SBasis> B = B_handles.asBezier();
    cairo_md_sb(cr, B);
    vector<double> Asects, Bsects;
    g_cr = cr;
    //if(0) pair_intersect(Asects, Bsects, A, 0, 1, 
    //               B, 0, 1);
    
    
    intersect_steps = 0;
    
    vector<Geom::Point> Ab = A_handles.pts, Bb = B_handles.pts;
    std::vector<std::pair<double, double> > section = 
        find_intersections( A_handles.pts, B_handles.pts);
    cairo_stroke(cr);
    cairo_set_source_rgba (cr, 1., 0., 0, 0.8);
    for(unsigned i = 0; i < section.size(); i++) {
        draw_handle(cr, A(section[i].first));
    }
    cairo_stroke(cr);
    
    *notify << "total intersections: " << section.size() << std::endl;
    *notify << "steps to find: " << intersect_steps;
    
    Toy::draw(cr, notify, width, height, save);
}
public:
    PairIntersect (unsigned A_bez_ord, unsigned B_bez_ord) {
        handles.push_back(&A_handles);
        handles.push_back(&B_handles);
        A_handles.name = "A";
        B_handles.name = "B";
    for(unsigned i = 0; i < A_bez_ord; i++)
        A_handles.push_back(uniform()*400, uniform()*400);
    for(unsigned i = 0; i < B_bez_ord; i++)
        B_handles.push_back(uniform()*400, uniform()*400);
}
};

int main(int argc, char **argv) {
unsigned A_bez_ord=10;
unsigned B_bez_ord=3;
    if(argc > 2)
        sscanf(argv[2], "%d", &B_bez_ord);
    if(argc > 1)
        sscanf(argv[1], "%d", &A_bez_ord);
    init(argc, argv, new PairIntersect(A_bez_ord, B_bez_ord));

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
