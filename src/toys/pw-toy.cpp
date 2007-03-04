#include "pw-sb.h"

#include "path-cairo.h"
#include <iterator>
#include "toy-framework.cpp"
#include "bezier-to-sbasis.h"
#include <vector>

using namespace Geom;
using namespace std;

void cairo_pw(cairo_t *cr, pw_sb p) {
    for(int i = 0; i < p.size(); i++) {
        MultidimSBasis<2> B;
        B[0] = BezOrd(p.cuts[i], p.cuts[i+1]);
        B[1] = BezOrd(150) + p[i];
        cairo_md_sb(cr, B);
    }
}

void cairo_horiz(cairo_t *cr, vector<double> p) {
    for(int i = 0; i < p.size(); i++) {
        cairo_move_to(cr, p[i], 500);
        cairo_rel_line_to(cr, 0, 10);
    }
}

class PwToy: public Toy {
    unsigned segs, handles_per_curve, curves;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_source_rgba (cr, 0., 0.5, 0, 1);
        cairo_set_line_width (cr, 1);
        if(!save) {
            cairo_move_to(cr, handles[0]);
            for(int a = 0; a < curves; a++) {
                unsigned base = a*handles_per_curve;
                for(int i = 0; i < handles_per_curve; i+=4) {
                    if(i)
                        handles[i+base-1][0] = handles[i+base][0];
                }
                for(int i = 0; i < handles_per_curve; i+=4) {
                    for(int j = 1; j < 3; j++)
                        handles[i+base+j][0] = (1 - j*0.25)*handles[i+base][0] + (j*0.25)*handles[i+base+3][0];
                    //cairo_line_to(cr, handles[i]);
                }
            }
        }
        
        pw_sb pws[curves];
        for(int a = 0; a < curves; a++) {
            unsigned base = a * handles_per_curve;
            for(int i = 0; i < handles_per_curve; i+=4) {
                pws[a].push_cut(handles[i+base][0]);
                //Bad hack to move 0 to 150
                for(int j = base + i; j < base + i + 4; j++) handles[j] = Point(handles[j][0], handles[j][1] - 150);
                pws[a].push_seg( Geom::bezier_to_sbasis<2,3>(handles.begin()+i+base)[1] );
                for(int j = base + i; j < base + i + 4; j++) handles[j] = Point(handles[j][0], handles[j][1] + 150);
            }
            pws[a].push_cut(handles[base + handles_per_curve - 1][0]);
            assert(pws[a].invariants());
            
            cairo_pw(cr, pws[a]);
        }
        /*vector<double> new_cuts;
        new_cuts.push_back(50);
        new_cuts.push_back(175);
        new_cuts.push_back(550);
        pw_sb pw_out = partition(pws[0], new_cuts);
        cairo_horiz(cr, pw_out.cuts);
        assert(pw_out.invariants()); */
        cairo_pw(cr, pws[0] + pws[1]);
        cairo_stroke(cr);
        
       /* cairo_horiz(cr, roots(pws[0]));
        pw_sb pw_out = portion(pws[0], handles[handles.size() - 2][0], handles[handles.size() - 1][0]);
        cairo_set_source_rgba (cr, 0, .5, 0, .25);
        cairo_set_line_width(cr, 3);
        cairo_pw(cr, pw_out);
        cairo_stroke(cr);
        cairo_horiz(cr, pw_out.cuts);
*/
        *notify << pws[0].segn(handles[handles.size() - 1][0]) << "; " << pws[0].segt(handles[handles.size() - 1][0]);
        Toy::draw(cr, notify, width, height, save);
    }

    bool should_draw_numbers() { return false; }
        
    public:
    PwToy () {
        segs = 3;
        handles_per_curve = 4 * segs;
        curves = 2;
        for(int a = 0; a < curves; a++)
            for(unsigned i = 0; i < handles_per_curve; i++)
                handles.push_back(Point(150 + 300*i/(4*segs), uniform() * 150 + 150 - 150 * a));
        handles.push_back(Point(100, 400));
        handles.push_back(Point(300, 400));
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
