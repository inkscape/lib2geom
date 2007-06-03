#include "circle-circle.cpp"
#include "toy-framework.cpp"

using namespace Geom;

class CircleIntersect : public Toy {
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        double r1 = distance(handles[0], handles[1]);
        double r2 = distance(handles[2], handles[3]);
        Geom::Point i1, i2;

        cairo_set_line_width(cr,1.0);
        cairo_set_source_rgba(cr, 0.5, 0.5, 0.5, 0.5);
        cairo_arc(cr, handles[0][0], handles[0][1], r1, 0, M_PI*2);
        cairo_stroke(cr);
        cairo_arc(cr, handles[2][0], handles[2][1], r2, 0, M_PI*2);
        cairo_stroke(cr);
        
        switch (circle_circle_intersection(handles[0], r1, handles[2], r2, i1, i2)) {
        case 0: *notify << "No intersection"; break;
        case 1: *notify << "Containment"; break;
        case 2: *notify << i1 << " and " << i2;
            draw_handle(cr, i1); draw_handle(cr, i2);
            cairo_set_source_rgba(cr, 1, 0, 0, 1);
            cairo_stroke(cr);
        }
        
        
        Toy::draw(cr, notify, width, height, save);
    }

    public:
    CircleIntersect() {
        handles.push_back(Geom::Point(200,200)); handles.push_back(Geom::Point(250,200));
        handles.push_back(Geom::Point(150,150)); handles.push_back(Geom::Point(250,150));
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
