#include "s-basis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "d2.h"

#include "path-cairo.h"

#include "toy-framework.h"

using std::vector;
using namespace Geom;

class Clothoid: public Toy {
    public:
    Clothoid() {
        handles.push_back(Geom::Point(100, 400));
        handles.push_back(Geom::Point(400, 400));
    }

    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_source_rgba (cr, 0., 0., 0, 0.8);
        cairo_set_line_width (cr, 0.5);
        cairo_stroke(cr);
        
        cairo_set_source_rgba (cr, 0., 0.5, 0, 0.8);
        double a0 = ((handles[0][0]-width/4)*2)/width;
        double a1 = ((handles[1][0]-width/4)*2)/width;
        *notify << "[" << a0 << ", " << a1 << "]";
        
        D2<SBasis> B;
        Linear bo = Linear(a0*6,a1*6);
        SBasis t2 = Linear(0,1);
        t2 = t2*t2;
        B[0] = compose(cos(bo,6), t2);
        B[1] = compose(sin(bo,6), t2);
        B = integral(B);
        B -= B(0);
        cairo_md_sb(cr, B*300 + Point(200, 200));
        Toy::draw(cr, notify, width, height, save);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, "clothoid", new Clothoid());
    
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
