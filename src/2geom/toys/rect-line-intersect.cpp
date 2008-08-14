#include <2geom/basic-intersection.h>
#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/bezier-to-sbasis.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

using std::vector;
using namespace Geom;

#include <stdlib.h>
#include <stdio.h>

class RectLineIntersect: public Toy {
    PointSetHandle line_handles;
    PointSetHandle rect_handles;

    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_line_width (cr, 0.5);

        Point E(line_handles.pts[0]);
        Point F(line_handles.pts[1]);
        draw_line_seg(cr, E, F);

        Point A(rect_handles.pts[0]);
        Point C(rect_handles.pts[1]);

        Point B(A[X], C[Y]);
        Point D(C[X], A[Y]);

	cairo_set_source_rgba (cr, 0.0, 0.0, 1.0, 1);
        draw_line_seg(cr, A, B);
        draw_line_seg(cr, B, C);
        draw_line_seg(cr, C, D);
        draw_line_seg(cr, D, A);

        cairo_stroke(cr);

        std::vector<Point> intersections = rect_line_intersect(A, C, E, F);
        std::cout << "number of intersections: " << intersections.size() << std::endl;
        for (unsigned int i = 0; i < intersections.size(); ++i) {
            draw_handle(cr, intersections[i]);
            std::cout << "  " << intersections[i] << std::endl;
        }
        std::cout << std::endl;

        Toy::draw(cr, notify, width, height, save);
    }

public:
    RectLineIntersect () {
        handles.push_back(&line_handles);
        handles.push_back(&rect_handles);
        line_handles.name = "line";
        rect_handles.name = "rect";

        line_handles.push_back(Point(50,100));
        line_handles.push_back(Point(350,200));

        rect_handles.push_back(Point(100,100));
        rect_handles.push_back(Point(200,250));
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new RectLineIntersect());

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
