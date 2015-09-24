#include <2geom/basic-intersection.h>
#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/path-intersection.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>

using std::vector;
using namespace Geom;

class PairIntersect: public Toy {
    PointSetHandle A_handles;
    PointSetHandle B_handles;
    std::vector<Toggle> toggles;
    void mouse_pressed(GdkEventButton* e) {
        toggle_events(toggles, e);
        Toy::mouse_pressed(e);
    }
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
    
        draw_toggles(cr, toggles);
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
    
    cairo_save(cr);
    cairo_set_source_rgba(cr, 0, 0, 0, 1.0);
    cairo_set_line_width (cr, 0.5);
    SBasis crs (cross(A - A(0), derivative(A)));
    crs = shift(crs*Linear(-1, 0)*Linear(-1, 0), -2);
    crs = crs * (300/(*bounds_exact(crs)).extent());
    vector<double> rts = roots(crs);
    for(unsigned i = 0; i < rts.size(); i++) {
        double t = rts[i];
        cairo_move_to(cr, A(0));
        cairo_line_to(cr, A(t));
        cairo_stroke(cr);
    }
    cairo_restore(cr);
    cairo_move_to(cr, 0, 300);
    cairo_line_to(cr, width, 300);
    crs += 300;
    D2<SBasis > are_graph(SBasis(Linear(0, width)), crs );
    cairo_save(cr);
    cairo_d2_sb(cr, are_graph);
    cairo_set_line_width (cr, .5);
    cairo_set_source_rgba (cr, 0., 0., 0., 1);
    cairo_stroke(cr);
    cairo_restore(cr);
    
    Path PB;
    PB.append(B);
    Path PA;
    PA.append(A);
        
    if (toggles[0].on) {
        PathVector ps;
        ps.push_back(PA);
        ps.push_back(PB);
        CrossingSet cs = crossings_among(ps);
        *notify << "total intersections: " << cs.size() << '\n';
        cairo_stroke(cr);
        cairo_set_source_rgba (cr, 1., 0., 0, 0.8);
        for(unsigned i = 0; i < cs.size(); i++) {
            Crossings section = cs[i];
            *notify << "section " << i << ": " << section.size() << '\n';
            for(unsigned j = 0; j < section.size(); j++) {
                draw_handle(cr, A(section[j].ta));
                *notify << Geom::distance(A(section[j].ta), B(section[j].tb)) 
                        << std::endl;
            }
        }

        cairo_stroke(cr);
    } else {
        vector<Geom::Point> Ab = A_handles.pts, Bb = B_handles.pts;
        std::vector<std::pair<double, double> > section;
        find_intersections( section, A, B);
        std::vector<std::pair<double, double> > polished_section = section;
        *notify << "total intersections: " << section.size();
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
    }
    cairo_restore(cr);


    Toy::draw(cr, notify, width, height, save,timer_stream);
}
public:
    PairIntersect (unsigned A_bez_ord, unsigned B_bez_ord) {
        toggles.push_back(Toggle("Path", true));
        toggles[0].bounds = Rect(Point(10,100), Point(100, 130));
        //toggles.push_back(Toggle("Curve", true));
        //toggles[1].bounds = Rect(Point(10,130), Point(100, 160));
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
