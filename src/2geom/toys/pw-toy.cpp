#include <2geom/piecewise.h>
#include <2geom/sbasis.h>
#include <2geom/bezier-to-sbasis.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

#include <vector>

using namespace Geom;
using namespace std;

void cairo_pw(cairo_t *cr, Piecewise<SBasis> p) {
    for(unsigned i = 0; i < p.size(); i++) {
        D2<SBasis> B;
        B[0] = Linear(p.cuts[i], p.cuts[i+1]);
        B[1] = Linear(150) + p[i];
        cairo_md_sb(cr, B);
    }
}

void cairo_horiz(cairo_t *cr, double y, vector<double> p) {
    for(unsigned i = 0; i < p.size(); i++) {
        cairo_move_to(cr, p[i], y);
        cairo_rel_line_to(cr, 0, 10);
    }
}

void cairo_vert(cairo_t *cr, double x, vector<double> p) {
    for(unsigned i = 0; i < p.size(); i++) {
        cairo_move_to(cr, x, p[i]);
        cairo_rel_line_to(cr, 10, 0);
    }
}

#include <2geom/toys/pwsbhandle.cpp>  // FIXME: This looks like it may give problems later, (including a .cpp file)

class PwToy: public Toy {
    unsigned segs, handles_per_curve, curves;
    PWSBHandle pwsbh[2];
    PointHandle interval_test[2];
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_source_rgba (cr, 0., 0., 0., 1);
        cairo_set_line_width (cr, 1);
        
        Piecewise<SBasis> pws[curves];
        for(unsigned a = 0; a < curves; a++) {
            pws[a] = pwsbh[a].value();
            cairo_pw(cr, pws[a]);
        }
        cairo_stroke(cr);

        Piecewise<SBasis> pw_out = pws[0] + pws[1];

        cairo_set_source_rgba (cr, 0., 0., .5, 1.);
        cairo_horiz(cr, 500, pw_out.cuts);
        cairo_stroke(cr);

        cairo_set_source_rgba (cr, 0., 0., .5, 1.);
        cairo_pw(cr, pws[0] + pws[1]);
        cairo_stroke(cr);
        
        Interval bs = bounds_local(pw_out, Interval(interval_test[0].pos[0], 
                                                    interval_test[1].pos[0]));
        vector<double> vec;
        vec.push_back(bs.min() + 150); vec.push_back(bs.max() + 150);
        cairo_set_source_rgba (cr, .5, 0., 0., 1.);
        cairo_vert(cr, 100, vec);
        cairo_stroke(cr);

       /*  Portion demonstration
        Piecewise<SBasis> pw_out = portion(pws[0], handles[handles.size() - 2][0], handles[handles.size() - 1][0]);
        cairo_set_source_rgba (cr, 0, .5, 0, .25);
        cairo_set_line_width(cr, 3);
        cairo_pw(cr, pw_out);
        cairo_stroke(cr);
        */

        *notify << pws[0].segN(interval_test[0].pos[0]) << "; " << pws[0].segT(interval_test[0].pos[0]);
        Toy::draw(cr, notify, width, height, save);
    }

    bool should_draw_numbers() { return false; }
        
    public:
    PwToy () {
        segs = 3;
        handles_per_curve = 4 * segs;
        curves = 2;
        for(unsigned a = 0; a < curves; a++) {
            pwsbh[a] = PWSBHandle(4, 3);
            handles.push_back(&pwsbh[a]);
            for(unsigned i = 0; i < handles_per_curve; i++)
                pwsbh[a].push_back(150 + 300*i/(4*segs), uniform() * 150 + 150 - 50 * a);
        }
        interval_test[0].pos = Point(150, 400);
        interval_test[1].pos = Point(300, 400);
        handles.push_back(&interval_test[0]);
        handles.push_back(&interval_test[1]);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new PwToy());
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
