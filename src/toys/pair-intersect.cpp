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
const unsigned bez_ord = 5;
using namespace Geom;
cairo_t *g_cr = 0;


/** Given two linear md_sb(assume they are linear even if they're not)
    find the ts at the intersection. */
void
linear_pair_intersect(MultidimSBasis<2> A, double Al, double Ah, 
                      MultidimSBasis<2> B, double Bl, double Bh,
                      double tA, double tB) {
    // kramers rule here
}

void pair_intersect(vector<double> &Asects,
                    vector<double> &Bsects,
                    MultidimSBasis<2> A, double Al, double Ah, 
                    MultidimSBasis<2> B, double Bl, double Bh, int depth=0) {
    // we'll split only A, and swap args
    if(depth > 5)
        return;
    Rect Ar = local_bounds(A, Al, Ah);
    if(Ar.isEmpty()) return;
    cairo_rectangle(g_cr, Ar.min()[0], Ar.min()[1], Ar.max()[0], Ar.max()[1]);
    cairo_stroke(g_cr);

    Rect Br = local_bounds(B, Bl, Bh);
    if(Br.isEmpty()) return;
    
    if(Ar.intersects(Br)) {
        if((A.tail_error(2) < eps) && 
           (B.tail_error(2) < eps)) {
            
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
    
    MultidimSBasis<2> A = bezier_to_sbasis<2, bez_ord-1>(handles.begin());
    cairo_md_sb(cr, A);
    
    Rect Ar = local_bounds(A, 0, 1);
    
    MultidimSBasis<2> B = bezier_to_sbasis<2, bez_ord-1>(handles.begin()+bez_ord);
    cairo_md_sb(cr, B);
    
    vector<double> Asects, Bsects;
    g_cr = cr;
    pair_intersect(Asects, Bsects,
                    A, 0, 1, 
                   B, 0, 1);
    for(int i = 0; i < Asects.size(); i++) {
        draw_handle(cr, point_at(A, Asects[i]));
    }

    Toy::draw(cr, notify, width, height, save);
}
public:
PairIntersect () {
    for(int j = 0; j < 2; j++)
    for(unsigned i = 0; i < bez_ord; i++) handles.push_back(Geom::Point(uniform()*400, uniform()*400));
}
};

int main(int argc, char **argv) {   
    init(argc, argv, "Pair Intersect", new PairIntersect());

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
