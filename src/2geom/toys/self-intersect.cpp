#include "basic-intersection.h"
#include "d2.h"
#include "sbasis.h"
#include "sbasis-2d.h"
#include "bezier-to-sbasis.h"

#include "path-cairo.h"
#include "toy-framework-2.h"

using std::vector;
const unsigned bez_ord = 6;
using namespace Geom;

class SelfIntersect: public Toy {
    PointSetHandle psh;
virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
    cairo_set_line_width (cr, 0.5);
    cairo_set_source_rgba (cr, 0., 0., 0, 1);
    
    D2<SBasis> A = psh.asBezier();
    Rect Ar = bounds_fast(A);
    cairo_md_sb(cr, A);
    cairo_stroke(cr);

    std::vector<std::pair<double, double> >  all_si = 
        find_self_intersections(A);
    
    cairo_stroke(cr);
    cairo_set_source_rgba (cr, 1., 0., 1, 1);
    for(unsigned i = 0; i < all_si.size(); i++) {
        draw_handle(cr, A(all_si[i].first));
    }
    cairo_stroke(cr);
    
    *notify << "total intersections: " << all_si.size();

    Toy::draw(cr, notify, width, height, save);
}
public:
SelfIntersect () {
    handles.push_back(&psh);
    for(unsigned i = 0; i < bez_ord; i++)
        psh.push_back(uniform()*400, uniform()*400);
}
};

int main(int argc, char **argv) {   
    init(argc, argv, new SelfIntersect());

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
