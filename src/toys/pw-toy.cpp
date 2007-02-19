#include "sb-pw.h"

#include "path-cairo.h"
#include <iterator>
#include "toy-framework.cpp"
#include "bezier-to-sbasis.h"

using namespace Geom;

int segs;

void cairo_pw(cairo_t *cr, pw_sb p, int start, int width) {
    int c = start;
    for(int i = 0; i < p.size(); i++) {
        MultidimSBasis<2> B;
        B[0] = BezOrd(c, c + width);
        B[1] = p[i];
        c += width;
        cairo_md_sb(cr, B);
    }
}

class PwToy: public Toy {    
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_source_rgba (cr, 0., 0.5, 0, 1);
        cairo_set_line_width (cr, 1);
        if(!save) {
            handles[0][0] = 150;
            cairo_move_to(cr, handles[0]);
            for(int i = 1; i < handles.size(); i++) {
                handles[i][0] = handles[i-1][0] + (i % 4 ? 100 / segs : 0);
                //cairo_line_to(cr, handles[i]);
            }
        }
        
        pw_sb pw;
        for(int i = 0; i < handles.size(); i+=4) {
            pw.cuts.push_back(i);
            SBasis foo = Geom::bezier_to_sbasis<2,3>(handles.begin()+i)[1];
            pw.segs.push_back(foo);
        }
        pw.cuts.push_back(handles.size());
        
        cairo_pw(cr, pw, 150, 100 / segs * 3);
        
        Toy::draw(cr, notify, width, height, save);
    }

    public:
    PwToy () {
        segs = 3;
        for(unsigned i = 0; i < 4 * segs; i++)
            handles.push_back(Point(0, uniform() * 150 + 300));
    }
};

int main(int argc, char **argv) {
    init(argc, argv, "Piecewise Toy", new PwToy());
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
