#include "sb-pw.h"

#include "path-cairo.h"
#include <iterator>
#include "toy-framework.cpp"
#include "bezier-to-sbasis.h"

using namespace Geom;

int segs;

void cairo_pw(cairo_t *cr, pw_sb p) {
    for(int i = 0; i < p.size(); i++) {
        MultidimSBasis<2> B;
        B[0] = BezOrd(p.cuts[i], p.cuts[i+1]);
        B[1] = p[i];
        cairo_md_sb(cr, B);
    }
}


void cairo_pw_cuts(cairo_t *cr, pw_sb p) {
    for(int i = 0; i < p.cuts.size(); i++) {
        cairo_move_to(cr, p.cuts[i], 500);
        cairo_rel_line_to(cr, 0, 10);
    }
}

class PwToy: public Toy {
    unsigned handles_per_curve;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_source_rgba (cr, 0., 0.5, 0, 1);
        cairo_set_line_width (cr, 1);
        if(!save) {
            cairo_move_to(cr, handles[0]);
            for(int a = 0; a < 2; a++) {
                unsigned base = a*handles_per_curve;
                for(int i = 0; i < handles_per_curve; i+=4) {
                    if(i)
                        handles[i+base-1][0] = handles[i+base][0];
                    for(int j = 1; j < 3; j++)
                        handles[i+base+j][0] = (1 - j*0.25)*handles[i+base][0] + (j*0.25)*handles[i+base+3][0];
                    //cairo_line_to(cr, handles[i]);
                }
            }
        }
        
        pw_sb pws[2];
        for(int a = 0; a < 1; a++) {
            unsigned base = a*handles_per_curve;
            for(int i = 0; i < handles_per_curve; i+=4) {
                pws[a].cuts.push_back(handles[i+base][0]);
                SBasis foo = Geom::bezier_to_sbasis<2,3>(handles.begin()+i+base)[1];
                pws[a].segs.push_back(foo);
            }
            pws[a].cuts.push_back(handles.back()[0]);
            assert(pws[a].cheap_invariants());
            
            cairo_pw(cr, pws[a]);
        }
        vector<double> new_cuts;
        new_cuts.push_back(50);
        new_cuts.push_back(550);
        pw_sb pw_out = partition(pws[0], new_cuts);
        cairo_pw_cuts(cr, pw_out);
        //cairo_pw(cr, pw_out);
        
        
        Toy::draw(cr, notify, width, height, save);
    }

    public:
    PwToy () {
        segs = 3;
        handles_per_curve = 4 * segs;
        for(int a = 0; a < 2; a++)
            for(unsigned i = 0; i < 4 * segs; i++)
                handles.push_back(Point(150 + 300*i/(4*segs), uniform() * 150 + 300));
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
