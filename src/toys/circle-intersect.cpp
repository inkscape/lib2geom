#include <toys/toy-framework-2.h>
#include <2geom/circle.h>

using namespace Geom;

class CircleIntersect : public Toy {
    PointSetHandle psh[2];
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        double r1 = Geom::distance(psh[0].pts[0], psh[0].pts[1]);
        double r2 = Geom::distance(psh[1].pts[0], psh[1].pts[1]);

        Circle c1(psh[0].pts[0], r1);
        Circle c2(psh[1].pts[0], r2);

        std::vector<ShapeIntersection> result = c1.intersect(c2);

        cairo_set_line_width(cr, 1.0);

        // draw the circles
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_arc(cr, c1.center(X), c1.center(Y), c1.radius(), 0, 2*M_PI);
        cairo_stroke(cr);
        cairo_arc(cr, c2.center(X), c2.center(Y), c2.radius(), 0, 2*M_PI);
        cairo_stroke(cr);

        // draw intersection points
        cairo_set_source_rgb(cr, 1, 0, 0);
        for (unsigned i = 0; i < result.size(); ++i) {
            draw_handle(cr, result[i].point());
        }
        cairo_stroke(cr);

        // show message
        if (c1.contains(c2)) {
            *notify << "Containment";
        } else if (!result.empty()) {
            for (unsigned i = 0; i < result.size(); ++i) {
                *notify << result[i].point() << "  ";
            }
        } else {
            *notify << "No intersection";
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
