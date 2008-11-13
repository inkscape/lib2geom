#include <2geom/basic-intersection.h>
#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/bezier-to-sbasis.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

using std::vector;
using namespace Geom;

class PairIntersect: public Toy {
    PointSetHandle A_handles;
    PointSetHandle B_handles;
virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
    cairo_save(cr);
    cairo_set_source_rgba(cr, 0, 0, 0, 1.0);
    cairo_set_line_width (cr, 0.5);
    D2<SBasis> A = A_handles.asBezier();
    cairo_d2_sb(cr, A);
    cairo_stroke(cr);
    cairo_set_source_rgba(cr, 0.0, 0, 0.8, 1.0);
    cairo_set_line_width (cr, 0.5);
    D2<SBasis> B = B_handles.asBezier();
    cairo_d2_sb(cr, B);
    cairo_stroke(cr);

    vector<Geom::Point> Ab = A_handles.pts, Bb = B_handles.pts;
    std::vector<std::pair<double, double> > section;
    find_intersections( section, A, B);
    std::vector<std::pair<double, double> > polished_section = section;
    polish_intersections( polished_section, A, B);
    cairo_stroke(cr);
    cairo_set_source_rgba (cr, 1., 0., 0, 0.8);
    for(unsigned i = 0; i < section.size(); i++) {
        draw_handle(cr, A(section[i].first));
        *notify << Geom::distance(A(section[i].first), B(section[i].second)) 
                << " polished "
                << Geom::distance(A(polished_section[i].first), B(polished_section[i].second)) 
                << std::endl;
    }

    cairo_stroke(cr);
    cairo_restore(cr);

    *notify << "total intersections: " << section.size();

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
