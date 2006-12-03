#include "maybe.h"

#include "toy-framework.cpp"

using std::vector;

void draw_rat_bez(cairo_t *cr, Geom::Point* h, double w) {
    cairo_move_to(cr, h[0]);
    for(int i = 1; i <= 100; i++) {
        double t = i*0.01;
        //TODO: use bezier-utils
        Geom::Point p = (1-t)*(1-t)*h[0] + 
            2*(1-t)*t*w*h[1] + 
            t*t*h[2];
        double pw = (1-t)*(1-t) + 
            2*(1-t)*t*w + 
            t*t;
        cairo_line_to(cr, p/pw);
    }
    cairo_stroke(cr);
}

class MyToy: public Toy {
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_line_width (cr, 1);

        for(int i = 0; i < 4; i++)
            draw_line_seg(cr, handles[i], handles[i+1]);
        Geom::Point h2[3] = {handles[0], handles[1], (handles[1] + handles[2])/2};
        Geom::Point A = unit_vector(h2[0]-h2[1]);
        Geom::Point B = unit_vector(h2[2]-h2[1]);
        draw_rat_bez(cr, h2, dot(A, B));
        h2[0] = h2[2];
        h2[1] = handles[2];
        h2[2] = (handles[2] + handles[3])/2;
        A = unit_vector(h2[0]-h2[1]);
        B = unit_vector(h2[2]-h2[1]);
        draw_rat_bez(cr, h2, dot(A, B));
        h2[0] = h2[2];
        h2[1] = handles[3];
        h2[2] = handles[4];
        A = unit_vector(h2[0]-h2[1]);
        B = unit_vector(h2[2]-h2[1]);
        draw_rat_bez(cr, h2, dot(A, B));

        Toy::draw(cr, notify, width, height, save);
    }

    public:
    MyToy () {
        double sqrt3 = sqrt(3);
        handles.push_back(Geom::Point(220, 20+sqrt3*200));
        handles.push_back(Geom::Point(20,  20+sqrt3*200));
        handles.push_back(Geom::Point(220, 20));
        handles.push_back(Geom::Point(420, 20+sqrt3*200));
        handles.push_back(Geom::Point(220, 20+sqrt3*200));
    }
};

int main(int argc, char **argv) {
    init(argc, argv, "untitled", new MyToy());
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
