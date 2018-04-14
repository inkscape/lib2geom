#include <2geom/cairo-path-sink.h>
#include <2geom/ellipse.h>
#include <2geom/line.h>
#include <2geom/polynomial.h>
#include <toys/toy-framework-2.h>

using namespace Geom;

class CircleIntersect : public Toy {
    PointSetHandle eh, lh;
    Ellipse e;
    Line l;

    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) override {

        Rect all(Point(0,0), Point(width, height));

        double rx = Geom::distance(eh.pts[0], eh.pts[1]);
        double ry = Geom::distance(eh.pts[0], eh.pts[2]);
        double rot = Geom::atan2(eh.pts[1] - eh.pts[0]);

        Ellipse e(eh.pts[0], Point(rx, ry), rot);
        LineSegment l(lh.pts[0], lh.pts[1]);

        cairo_set_line_width(cr, 1.0);

        cairo_set_source_rgb(cr, 0, 0, 0);
        draw_line_segment(cr, l, all);
        Geom::CairoPathSink cps(cr);
        cps.feed(e);
        cairo_stroke(cr);

        std::vector<ShapeIntersection> result = e.intersect(l);

        cairo_set_source_rgb(cr, 1, 0, 0);
        for (unsigned i = 0; i < result.size(); ++i) {
            draw_handle(cr, result[i].point());
        }
        cairo_stroke(cr);

        Toy::draw(cr, notify, width, height, save,timer_stream);
    }

    public:
    CircleIntersect(){
        eh.push_back(300,300); eh.push_back(450,150); eh.push_back(250, 350);
        lh.push_back(280, 50); lh.push_back(320,550);
        handles.push_back(&eh);
        handles.push_back(&lh);
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
