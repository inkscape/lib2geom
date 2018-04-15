#include <2geom/basic-intersection.h>
#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-2d.h>
#include <2geom/bezier-to-sbasis.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>

using std::vector;
using namespace Geom;

class SelfIntersect: public Toy {
    PointSetHandle psh;
void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) override {
    cairo_set_line_width (cr, 0.5);
    cairo_set_source_rgba (cr, 0., 0., 0, 1);
    
    D2<SBasis> A = psh.asBezier();
    //Rect Ar = *bounds_fast(A);
    cairo_d2_sb(cr, A);
    cairo_stroke(cr);

    std::vector<std::pair<double, double> >  all_si;
    
    find_self_intersections(all_si, A);
    
    cairo_stroke(cr);
    cairo_set_source_rgba (cr, 1., 0., 1, 1);
    for(auto & si : all_si) {
        draw_handle(cr, A(si.first));
    }
    cairo_stroke(cr);
    
    *notify << "total intersections: " << all_si.size();

    Toy::draw(cr, notify, width, height, save,timer_stream);
}
public:
SelfIntersect (unsigned bez_ord) {
    handles.push_back(&psh);
    for(unsigned i = 0; i < bez_ord; i++)
        psh.push_back(uniform()*400, uniform()*400);
}
};

int main(int argc, char **argv) {   
    unsigned bez_ord=5;
    if(argc > 1)
        sscanf(argv[1], "%d", &bez_ord);
    init(argc, argv, new SelfIntersect(bez_ord));

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
