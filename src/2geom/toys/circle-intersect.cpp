#include <2geom/circle-circle.cpp>  // FIXME: This looks like it may give problems later, (including a .cpp file)
#include <2geom/toys/toy-framework-2.h>

using namespace Geom;

class CircleIntersect : public Toy {
    PointSetHandle psh[2];
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        double r1 = Geom::distance(psh[0].pts[0], psh[0].pts[1]);
        double r2 = Geom::distance(psh[1].pts[0], psh[1].pts[1]);
        Geom::Point i1, i2;

        cairo_set_line_width(cr,1.0);
        cairo_set_source_rgba(cr, 0.5, 0.5, 0.5, 0.5);
        cairo_arc(cr, psh[0].pts[0][0], psh[0].pts[0][1], r1, 0, M_PI*2);
        cairo_stroke(cr);
        cairo_arc(cr, psh[1].pts[0][0], psh[1].pts[0][1], r2, 0, M_PI*2);
        cairo_stroke(cr);
        
        switch (circle_circle_intersection(psh[0].pts[0], r1, psh[1].pts[0], r2, i1, i2)) {
        case 0: *notify << "No intersection"; break;
        case 1: *notify << "Containment"; break;
        case 2: *notify << i1 << " and " << i2;
            draw_handle(cr, i1); draw_handle(cr, i2);
            cairo_set_source_rgba(cr, 1, 0, 0, 1);
            cairo_stroke(cr);
        }
        
        Toy::draw(cr, notify, width, height, save,timer_stream);
    }

    public:
    CircleIntersect(){
        psh[0].push_back(200,200); psh[0].push_back(250,200);
        psh[1].push_back(150,150); psh[1].push_back(250,150);
        handles.push_back(&psh[0]);
        handles.push_back(&psh[1]);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new CircleIntersect());
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:fileencoding=utf-8:textwidth=99 :
