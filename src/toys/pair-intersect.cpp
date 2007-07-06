#include "basic-intersection.h"
#include "d2.h"
#include "sbasis.h"
#include "bezier-to-sbasis.h"

#include "path-cairo.h"
#include "toy-framework.h"

using std::vector;
using namespace Geom;

const unsigned bez_ord = 10;
cairo_t *g_cr = 0;
const double eps = 0.1;

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

class PairIntersect: public Toy {
virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
    cairo_set_line_width (cr, 0.5);
    
    D2<SBasis> A = handles_to_sbasis<bez_ord-1>(handles.begin());
    cairo_md_sb(cr, A);
    
    D2<SBasis> B = handles_to_sbasis<bez_ord-1>(handles.begin()+bez_ord);
    cairo_md_sb(cr, B);
    vector<double> Asects, Bsects;
    g_cr = cr;
    if(0) pair_intersect(Asects, Bsects, A, 0, 1, 
                   B, 0, 1);
    
    
    vector<Geom::Point> Ab, Bb;
    Ab.insert(Ab.begin(), handles.begin(), handles.begin()+bez_ord);
    Bb.insert(Bb.begin(), handles.begin()+bez_ord, handles.begin()+2*bez_ord);
    std::vector<std::pair<double, double> > section = 
        find_intersections( Ab, Bb);
    cairo_stroke(cr);
    cairo_set_source_rgba (cr, 1., 0., 0, 0.8);
    for(unsigned i = 0; i < section.size(); i++) {
        draw_handle(cr, A(section[i].first));
    }
    cairo_stroke(cr);
    
    *notify << "total intersections: " << section.size();
    
    Toy::draw(cr, notify, width, height, save);
}
public:
PairIntersect () {
    for(unsigned j = 0; j < 2; j++)
    for(unsigned i = 0; i < bez_ord; i++) handles.push_back(Geom::Point(uniform()*400, uniform()*400));
}
};

int main(int argc, char **argv) {   
    init(argc, argv, new PairIntersect());

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
