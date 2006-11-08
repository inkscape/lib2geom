#include "s-basis.h"
#include "sbasis-poly.h"

#include "toy-framework.cpp"

using std::string;
using std::vector;

BezOrd z0(0.5,1.);

class OneD: public Toy {
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_source_rgba (cr, 0., 0., 0, 0.8);
        cairo_set_line_width (cr, 0.5);
        cairo_stroke(cr);
        SBasis one(BezOrd(1,1));
        assert(one.size() == 1);
        SBasis P0(z0), P1(BezOrd(3, 1));
        SBasis Q = multiply(P0, P1);
        Q = multiply(Q, P1);
        cairo_set_source_rgba (cr, 0.5, 0., 0,1);
        Poly qp = sbasis_to_poly(Q);
        qp.normalize();
        std::cout << qp << std::endl;
        Q = integral(Q);
        qp = derivative(sbasis_to_poly(Q));
        qp.normalize();
        std::cout << qp << std::endl;
        for(int ti = 0; ti < width; ti++) {
            double t = (double(ti))/(width);
            double y = Q(t);
            if(ti)
                cairo_line_to(cr, t*width/2 + width/4, 3*height/4 - y*height/2);
            else
                cairo_move_to(cr, t*width/2 + width/4, 3*height/4 - y*height/2);
        }    
    }

    virtual void mouse_pressed(GdkEventButton* e) {
        if(e->button == 1)
            z0[1] /= 0.9;
        else if(e->button == 3)
            z0[1] *= 0.9;
        redraw();
    }
};

int main(int argc, char **argv) {
    init(argc, argv, "one-D", new OneD());

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
